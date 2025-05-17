#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 2048

// Prints the menu options based on whether the user is authenticated or not
void print_menu(int authenticated) {
    printf("\nMenu:\n");
    if(!authenticated) {
        // Options for unauthenticated users
        printf("1. Register\n");
        printf("2. Login\n");
    } else {
        // Options available after login
        printf("3. Post Question\n");
        printf("4. List Questions\n");
        printf("5. Answer Question\n");
        printf("6. Search Questions\n");
        printf("7. Rate Answer\n");
        printf("8. View Leaderboard\n");
    }
    printf("9. Exit\n");
    printf("Choice: ");
}

// Displays a list of all available questions retrieved from the server
void display_questions(const char *buffer) {
    printf("\n--- Questions ---\n");

    // Skip the status header (e.g., "OK|") and get to the data payload
    char *payload = strchr(buffer, '|') + 1;

    // Tokenize by ';' to get individual question records
    char *token = strtok(payload, ";");

    while(token) {
        // Parse question details (ID, question text, author, answer count)
        char *saveptr;
        char *id = strtok_r(token, "|", &saveptr);
        char *question = strtok_r(NULL, "|", &saveptr);
        char *author = strtok_r(NULL, "|", &saveptr);
        char *answers = strtok_r(NULL, "|", &saveptr);

        if(id && question && author && answers) {
            printf("[%s] %s\n   Asked by: %s (%s answers)\n", id, question, author, answers);
        }

        token = strtok(NULL, ";"); // Proceed to the next question
    }
}

// Displays search result for a specific question including answers
void display_search_results(const char *buffer) {
    char *status = strtok((char *)buffer, "|");       // e.g., OK or ERROR
    char *question = strtok(NULL, "|");               // Question text or "Question not found"
    char *answers = strtok(NULL, "|");                // Answers or "No answers yet"

    if(strcmp(question, "Question not found") == 0) {
        printf("\nNo matching questions found\n");
        return;
    }

    printf("\nQuestion: %s\n", question);
    if(strcmp(answers, "No answers yet") == 0) {
        printf("Answers: %s\n", answers);
    } else {
        // Tokenize multiple answers
        printf("Answers:\n");
        char *answer = strtok(answers, ";");
        while(answer) {
            printf("- %s\n", answer);
            answer = strtok(NULL, ";");
        }
    }
}

int main() {
    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    int authenticated = 0;
    char username[50] = {0};

    // Set server address details
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert localhost IP string to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }

    // Connect to the server
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    // Main application loop
    while(1) {
        print_menu(authenticated);

        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline from input buffer

        switch(choice) {
            // -------------------- Register --------------------
            case 1: {
                if(authenticated) break;

                char user[50], pass[50];

                printf("Username: ");
                fgets(user, 50, stdin);
                user[strcspn(user, "\n")] = 0; // Remove newline

                printf("Password: ");
                fgets(pass, 50, stdin);
                pass[strcspn(pass, "\n")] = 0;

                // Format registration request
                snprintf(buffer, sizeof(buffer), "REGISTER|%s|%s", user, pass);
                send(sock, buffer, strlen(buffer), 0);

                // Get server response
                recv(sock, buffer, BUFFER_SIZE, 0);
                printf("Server: %s\n", strchr(buffer, '|') + 1);
                break;
            }

            // -------------------- Login --------------------
            case 2: {
                if(authenticated) break;

                char user[50], pass[50];
                printf("Username: ");
                fgets(user, 50, stdin);
                user[strcspn(user, "\n")] = 0;

                printf("Password: ");
                fgets(pass, 50, stdin);
                pass[strcspn(pass, "\n")] = 0;

                // Format login request
                snprintf(buffer, sizeof(buffer), "LOGIN|%s|%s", user, pass);
                send(sock, buffer, strlen(buffer), 0);

                // Receive response
                recv(sock, buffer, BUFFER_SIZE, 0);
                buffer[BUFFER_SIZE-1] = '\0';

                // Parse response to determine login status
                if(strncmp(buffer, "OK", 2) == 0) {
                    char *name = strchr(buffer, '|') + 1;
                    char *credits = strchr(name, '|') + 1;
                    *strchr(name, '|') = '\0';
                    printf("Welcome %s (Credits: %s)\n", name, credits);
                    strcpy(username, name);
                    authenticated = 1;
                } else {
                    printf("Error: %s\n", strchr(buffer, '|') + 1);
                }
                break;
            }

            // -------------------- Post a Question --------------------
            case 3: {
                if(!authenticated) break;

                char question[256];
                printf("Enter your question: ");
                fgets(question, 256, stdin);
                question[strcspn(question, "\n")] = 0;

                snprintf(buffer, sizeof(buffer), "POST|%s", question);
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, buffer, BUFFER_SIZE, 0);

                printf("Server: %s\n", strchr(buffer, '|') + 1);
                break;
            }

            // -------------------- List Questions --------------------
            case 4: {
                if(!authenticated) break;

                send(sock, "LISTQ", 5, 0);
                recv(sock, buffer, BUFFER_SIZE, 0);
                display_questions(buffer);
                break;
            }

            // -------------------- Answer a Question --------------------
            case 5: {
                if(!authenticated) break;

                char qnum[10], answer[256];
                printf("Enter question number: ");
                fgets(qnum, 10, stdin);
                qnum[strcspn(qnum, "\n")] = 0;

                printf("Enter your answer: ");
                fgets(answer, 256, stdin);
                answer[strcspn(answer, "\n")] = 0;

                snprintf(buffer, sizeof(buffer), "ANSWER|%s|%s", qnum, answer);
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, buffer, BUFFER_SIZE, 0);

                printf("Server: %s\n", strchr(buffer, '|') + 1);
                break;
            }

            // -------------------- Search Questions --------------------
            case 6: {
                if(!authenticated) break;

                char query[256];
                printf("Enter search query: ");
                fgets(query, 256, stdin);
                query[strcspn(query, "\n")] = 0;

                snprintf(buffer, sizeof(buffer), "SEARCH|%s", query);
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, buffer, BUFFER_SIZE, 0);

                display_search_results(buffer);
                break;
            }

            // -------------------- Rate an Answer --------------------
            case 7: {
                if(!authenticated) break;

                char qid[10], aid[10], rating[10];

                printf("Enter question ID: ");
                fgets(qid, 10, stdin);
                qid[strcspn(qid, "\n")] = 0;

                printf("Enter answer number (0-based index): ");
                fgets(aid, 10, stdin);
                aid[strcspn(aid, "\n")] = 0;

                printf("Enter rating (1-5): ");
                fgets(rating, 10, stdin);
                rating[strcspn(rating, "\n")] = 0;

                snprintf(buffer, sizeof(buffer), "RATE|%s|%s|%s", qid, aid, rating);
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, buffer, BUFFER_SIZE, 0);

                printf("Server: %s\n", strchr(buffer, '|') + 1);
                break;
            }

            // -------------------- View Leaderboard --------------------
            case 8: {
                if(!authenticated) break;

                snprintf(buffer, sizeof(buffer), "LEADER");
                send(sock, buffer, strlen(buffer), 0);
                recv(sock, buffer, BUFFER_SIZE, 0);

                printf("\n--- Leaderboard ---\n%s\n", strchr(buffer, '|') + 1);
                break;
            }

            // -------------------- Exit --------------------
            case 9:
                close(sock); // Close socket connection
                exit(0);     // Exit the program

            // -------------------- Invalid Choice --------------------
            default:
                printf("Invalid choice\n");
        }

        // Clear buffer after each interaction
        memset(buffer, 0, BUFFER_SIZE);
    }

    return 0;
}

