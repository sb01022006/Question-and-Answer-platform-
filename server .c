#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/sha.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048
#define MAX_USERS 100
#define MAX_QUESTIONS 1000
#define MAX_ANSWERS 20

// User record structure
typedef struct {
    char username[50];
    char password_hash[SHA256_DIGEST_LENGTH*2 + 1];
    int credits;       // credits for posting/questioning
    int is_manager;    // flag for future admin commands
    int score;         // cumulative score from rated answers
} User;

// Question + answers structure
typedef struct {
    char question[256];
    char author[50];
    char answers[MAX_ANSWERS][256];
    char answer_authors[MAX_ANSWERS][50];
    int answer_count;
    int ratings[MAX_ANSWERS];  // rating per answer
} Question;

// Per-connection session info
typedef struct {
    int sock;
    struct sockaddr_in addr;
    int user_idx;         // index into users[] array
    int authenticated;    // 0 = not logged in, 1 = logged in
} ClientSession;

// Global data and mutexes
User users[MAX_USERS];
Question questions[MAX_QUESTIONS];
int user_count = 0;
int question_count = 0;

pthread_mutex_t users_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t questions_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Hash a plaintext password using SHA-256 and output as hex string.
 */
void hash_password(const char *password, char *output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + i*2, "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH*2] = '\0';
}

/**
 * Load persisted users & questions from disk at startup.
 */
void load_data() {
    FILE *fp;
    if ((fp = fopen("users.dat", "rb"))) {
        fread(&user_count, sizeof(int), 1, fp);
        fread(users,       sizeof(User), user_count, fp);
        fclose(fp);
    }
    if ((fp = fopen("questions.dat", "rb"))) {
        fread(&question_count, sizeof(int), 1, fp);
        fread(questions,        sizeof(Question), question_count, fp);
        fclose(fp);
    }
}

/**
 * Save users array back to disk.
 */
void save_users() {
    FILE *fp = fopen("users.dat", "wb");
    if (!fp) return;
    fwrite(&user_count, sizeof(int), 1, fp);
    fwrite(users,       sizeof(User), user_count, fp);
    fclose(fp);
}

/**
 * Save questions array back to disk.
 */
void save_questions() {
    FILE *fp = fopen("questions.dat", "wb");
    fwrite(&question_count, sizeof(int), 1, fp);
    fwrite(questions,        sizeof(Question), question_count, fp);
    fclose(fp);
}

/**
 * Find a user by username; return index or -1 if not found.
 */
int find_user(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Send a simple status|message response back to client.
 */
void send_response(int sock, const char *status, const char *message) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s|%s", status, message);
    send(sock, buffer, strlen(buffer), 0);
}

/**
 * Handle REGISTER|username|password
 */
void handle_register(int sock, char *username, char *password) {
    pthread_mutex_lock(&users_mutex);

    if (find_user(username) != -1) {
        send_response(sock, "ERR", "Username exists");
        pthread_mutex_unlock(&users_mutex);
        return;
    }

    User new_user;
    strncpy(new_user.username, username, sizeof(new_user.username)-1);
    hash_password(password, new_user.password_hash);
    new_user.credits    = 100;   // starting credits
    new_user.is_manager = 0;
    new_user.score      = 0;

    users[user_count++] = new_user;
    save_users();

    pthread_mutex_unlock(&users_mutex);
    send_response(sock, "OK", "Registration successful");
}

/**
 * Handle LOGIN|username|password
 */
void handle_login(ClientSession *session, char *username, char *password) {
    pthread_mutex_lock(&users_mutex);

    int idx = find_user(username);
    if (idx < 0) {
        send_response(session->sock, "ERR", "User not found");
        pthread_mutex_unlock(&users_mutex);
        return;
    }

    char hash[SHA256_DIGEST_LENGTH*2 + 1];
    hash_password(password, hash);

    if (strcmp(users[idx].password_hash, hash) == 0) {
        session->user_idx      = idx;
        session->authenticated = 1;
        // Return OK|username|current_credits
        char resp[BUFFER_SIZE];
        snprintf(resp, sizeof(resp), "OK|%s|%d",
                 users[idx].username, users[idx].credits);
        send(session->sock, resp, strlen(resp), 0);
    } else {
        send_response(session->sock, "ERR", "Invalid password");
    }

    pthread_mutex_unlock(&users_mutex);
}

/**
 * Handle LOGOUT
 */
void handle_logout(ClientSession *session) {
    session->authenticated = 0;
    session->user_idx      = -1;
    send_response(session->sock, "OK", "Logged out");
}

/**
 * Handle POST|question_text
 */
void handle_post_question(ClientSession *session, char *question_text) {
    if (!session->authenticated) {
        send_response(session->sock, "ERR", "Not authenticated");
        return;
    }

    pthread_mutex_lock(&questions_mutex);
    if (question_count >= MAX_QUESTIONS) {
        send_response(session->sock, "ERR", "Question limit reached");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    // Add question
    Question *q = &questions[question_count];
    strncpy(q->question, question_text, sizeof(q->question)-1);
    strncpy(q->author, users[session->user_idx].username, sizeof(q->author)-1);
    q->answer_count = 0;
    memset(q->ratings, 0, sizeof(q->ratings));
    question_count++;

    // Reward credits
    pthread_mutex_lock(&users_mutex);
    users[session->user_idx].credits += 10;
    save_users();
    pthread_mutex_unlock(&users_mutex);

    save_questions();
    pthread_mutex_unlock(&questions_mutex);

    send_response(session->sock, "OK", "Question posted (+10 credits)");
}

/**
 * Handle ANSWER|question_index|answer_text
 */
void handle_answer(ClientSession *session, char *qidx_str, char *answer_text) {
    if (!session->authenticated) {
        send_response(session->sock, "ERR", "Not authenticated");
        return;
    }

    int qidx = atoi(qidx_str);
    pthread_mutex_lock(&questions_mutex);
    if (qidx < 0 || qidx >= question_count) {
        send_response(session->sock, "ERR", "Invalid question index");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    Question *q = &questions[qidx];
    if (q->answer_count >= MAX_ANSWERS) {
        send_response(session->sock, "ERR", "Answer limit reached");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    // Add answer
    int aidx = q->answer_count++;
    strncpy(q->answers[aidx], answer_text, sizeof(q->answers[aidx])-1);
    strncpy(q->answer_authors[aidx],
            users[session->user_idx].username,
            sizeof(q->answer_authors[aidx])-1);

    // Reward credits
    pthread_mutex_lock(&users_mutex);
    users[session->user_idx].credits += 5;
    save_users();
    pthread_mutex_unlock(&users_mutex);

    save_questions();
    pthread_mutex_unlock(&questions_mutex);

    send_response(session->sock, "OK", "Answer added (+5 credits)");
}

/**
 * Handle LISTQ
 * Returns: OK|idx|question|author|answer_count;...
 */
void handle_list_questions(ClientSession *session) {
    pthread_mutex_lock(&questions_mutex);

    char resp[BUFFER_SIZE] = "OK|";
    size_t pos = strlen(resp);

    for (int i = 0; i < question_count && pos < BUFFER_SIZE - 100; i++) {
        int n = snprintf(resp + pos, BUFFER_SIZE - pos,
                         "%d|%s|%s|%d;",
                         i,
                         questions[i].question,
                         questions[i].author,
                         questions[i].answer_count);
        pos += (n > 0 ? n : 0);
    }

    send(session->sock, resp, strlen(resp), 0);
    pthread_mutex_unlock(&questions_mutex);
}

/**
 * Handle SEARCH|keyword
 * Returns first matching question + answers or "Question not found"
 */
void handle_search(ClientSession *session, char *keyword) {
    if (!session->authenticated) {
        send_response(session->sock, "ERR", "Not authenticated");
        return;
    }

    pthread_mutex_lock(&questions_mutex);
    char resp[BUFFER_SIZE] = "OK|";
    int found = 0;

    for (int i = 0; i < question_count; i++) {
        if (strcasestr(questions[i].question, keyword)) {
            // Append question text
            strncat(resp, questions[i].question,
                    BUFFER_SIZE - strlen(resp) - 1);
            strncat(resp, "|", BUFFER_SIZE - strlen(resp) - 1);
            // Append answers or placeholder
            if (questions[i].answer_count > 0) {
                for (int j = 0; j < questions[i].answer_count; j++) {
                    strncat(resp, questions[i].answers[j],
                            BUFFER_SIZE - strlen(resp) - 1);
                    if (j < questions[i].answer_count - 1)
                        strncat(resp, ";", BUFFER_SIZE - strlen(resp) - 1);
                }
            } else {
                strncat(resp, "No answers yet",
                        BUFFER_SIZE - strlen(resp) - 1);
            }
            found = 1;
            break;
        }
    }

    if (!found) {
        strncat(resp, "Question not found",
                BUFFER_SIZE - strlen(resp) - 1);
    }

    send(session->sock, resp, strlen(resp), 0);
    pthread_mutex_unlock(&questions_mutex);
}

/**
 * Handle RATE|question_index|answer_index|score
 * Only the original question author may rate answers.
 */
void handle_rate_answer(ClientSession *session,
                        char *qidx_str,
                        char *aidx_str,
                        char *score_str)
{
    if (!session->authenticated) {
        send_response(session->sock, "ERR", "Not authenticated");
        return;
    }

    int qidx   = atoi(qidx_str);
    int aidx   = atoi(aidx_str);
    int score  = atoi(score_str);

    pthread_mutex_lock(&questions_mutex);

    // Validate indices
    if (qidx < 0 || qidx >= question_count ||
        aidx < 0 || aidx >= questions[qidx].answer_count)
    {
        send_response(session->sock, "ERR", "Invalid indices");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    // Only question's author can rate
    if (strcmp(users[session->user_idx].username,
               questions[qidx].author) != 0)
    {
        send_response(session->sock, "ERR", "Not the question author");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    // Find answer author in users[]
    int author_idx = find_user(questions[qidx].answer_authors[aidx]);
    if (author_idx < 0) {
        send_response(session->sock, "ERR", "Answer author not found");
        pthread_mutex_unlock(&questions_mutex);
        return;
    }

    // Record rating and update user score
    questions[qidx].ratings[aidx] = score;
    pthread_mutex_lock(&users_mutex);
    users[author_idx].score += score;
    save_users();
    pthread_mutex_unlock(&users_mutex);

    save_questions();
    pthread_mutex_unlock(&questions_mutex);

    send_response(session->sock, "OK", "Answer rated");
}

/**
 * Compare helper for sorting users by score descending.
 */
int compare_users(const void *a, const void *b) {
    return ((User *)b)->score - ((User *)a)->score;
}

/**
 * Handle LEADER
 * Returns top 10 users by cumulative score.
 */
void handle_leaderboard(ClientSession *session) {
    pthread_mutex_lock(&users_mutex);

    // Copy and sort
    User sorted[MAX_USERS];
    memcpy(sorted, users, sizeof(User) * user_count);
    qsort(sorted, user_count, sizeof(User), compare_users);

    char resp[BUFFER_SIZE] = "OK|\n--- Leaderboard ---\n";
    size_t pos = strlen(resp);
    pos += snprintf(resp + pos, BUFFER_SIZE - pos,
                    "%-5s %-20s %-6s\n",
                    "Rank", "Username", "Score");

    // Top 10 or fewer
    for (int i = 0; i < user_count && i < 10; i++) {
        pos += snprintf(resp + pos, BUFFER_SIZE - pos,
                        "%-5d %-20s %-6d\n",
                        i+1, sorted[i].username, sorted[i].score);
    }

    send(session->sock, resp, strlen(resp), 0);
    pthread_mutex_unlock(&users_mutex);
}

/**
 * Thread entrypoint: processes a single client's commands.
 */
void *client_handler(void *arg) {
    ClientSession *session = (ClientSession *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t len = recv(session->sock, buffer, sizeof(buffer)-1, 0);
        if (len <= 0) break;       // client disconnected
        buffer[len] = '\0';

        // Tokenize command and parameters by '|'
        char *cmd = strtok(buffer, "|");

        if      (strcmp(cmd, "REGISTER") == 0) {
            handle_register(session->sock,
                            strtok(NULL, "|"),
                            strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "LOGIN") == 0) {
            handle_login(session,
                         strtok(NULL, "|"),
                         strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "LOGOUT") == 0) {
            handle_logout(session);
        }
        else if (strcmp(cmd, "POST") == 0) {
            handle_post_question(session,
                                 strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "ANSWER") == 0) {
            handle_answer(session,
                          strtok(NULL, "|"),
                          strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "LISTQ") == 0) {
            handle_list_questions(session);
        }
        else if (strcmp(cmd, "SEARCH") == 0) {
            handle_search(session, strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "RATE") == 0) {
            handle_rate_answer(session,
                               strtok(NULL, "|"),
                               strtok(NULL, "|"),
                               strtok(NULL, "|"));
        }
        else if (strcmp(cmd, "LEADER") == 0) {
            handle_leaderboard(session);
        }
        else {
            send_response(session->sock, "ERR", "Unknown command");
        }
    }

    // Cleanup on disconnect
    close(session->sock);
    free(session);
    return NULL;
}

/**
 * Program entrypoint: initializes server socket, loads data,
 * then loops forever accepting clients and spawning threads.
 */
int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    load_data();

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to any interface on PORT
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening with backlog MAX_CLIENTS
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd,
                             (struct sockaddr *)&address,
                             &addrlen);
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }

        // Allocate session for new client
        ClientSession *session = malloc(sizeof(ClientSession));
        if (!session) {
            perror("malloc failed");
            close(client_sock);
            continue;
        }
        session->sock          = client_sock;
        session->addr          = address;
        session->user_idx      = -1;
        session->authenticated = 0;

        // Spawn thread and detach immediately
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, session) != 0) {
            perror("pthread_create failed");
            close(client_sock);
            free(session);
        } else {
            pthread_detach(tid);
        }
    }

    // Clean up (unreachable in this design)
    close(server_fd);
    return 0;
}

