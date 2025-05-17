# QUESTION AND ANSWER PLATFORM

Welcome to the **QUESTION AND ANSWER PLATFORM** repository! This platform is developed entirely in the **C programming language** and serves as a robust system for managing question-and-answer interactions. The repository is designed to be **efficient**, **scalable**, and **easy to extend**.

## Table of Contents

1. [PowerPoint Presentation](#powerpoint-presentation)
2. [Overview](#overview)
3. [Features](#features)
4. [Prerequisites](#prerequisites)
5. [Installation](#installation)
   - [Installation of OpenSSL Libraries (`-lssl` and `-lcrypto`)](#installing-openssl-libraries-lssl-and-lcrypto)
   - [Compiling and Running](#compiling-and-running)
6. [Usage](#usage)
7. [Features of `client.c`](#features-of-clientc)
8. [Features of `server.c`](#features-of-serverc)
9. [Commands and Structures in `client.c`](#commands-and-structures-in-clientc)
10. [Commands and Structures in `server.c`](#commands-and-structures-in-serverc)
11. [Working of the Client Side Process](#working-of-the-client-side-process)
12. [Project Responsibility](#project-responsibility)

## PowerPoint Presentation
[Question And Answer Platform Presentation](https://drive.google.com/file/d/1gycFTV6agJNMXrKvpIhSaJA8cc532mz0/view?usp=sharing)
<br>
This is the PowerPoint presentation of our Question and Answer Platform which includes complete explanation of the project and contains the modular level design of the project.

## Overview

The **QUESTION AND ANSWER PLATFORM** is a lightweight and efficient solution for handling Q&A functionalities. It is implemented entirely in the **C programming language**, providing high performance and portability. 

This platform is suitable for:
- **Educational purposes**: Facilitating knowledge-sharing in academic environments.
- **Community-driven Q&A forums**: Enabling users to ask and answer questions in an organized manner.
- **Technical support platforms**: Providing a structured way to handle user queries and responses.

## Features

- **Question Management**: Add, delete, and manage questions.
- **Answer Management**: Submit and view answers for questions.
- **Search Functionality**: Search for questions or answers based on keywords.
- **User Management**: Manage user profiles and activity (optional, if implemented in the future).
- **Lightweight**: Implemented in pure C for performance and efficiency.
- **Customizable**: Designed to be extended with additional features.

## Prerequisites

To build and run this platform, you will need:

1. A **C compiler** (e.g., `gcc`).
2. A **terminal** or command-line interface.
3. Basic knowledge of **C programming** and Makefiles (if using one).

## Installation of OpenSSL Libraries (`-lssl` and `-lcrypto`)

To build and run the **server** and **client** applications, you need to install the OpenSSL development libraries, which provide the `-lssl` and `-lcrypto` functionality for cryptographic operations like password hashing.

### 📦 Installing OpenSSL Libraries

#### On Debian/Ubuntu-based systems:

```bash
sudo apt update
sudo apt install libssl-dev
```

#### On RedHat/CentOS/Fedora-based systems, install the OpenSSL development package by running:

```bash
sudo dnf install openssl-devel
```

#### To install OpenSSL on Arch Linux, run:

```bash
sudo pacman -S openssl
```

#### Make sure you also have the GCC compiler and pthreads library available. Most systems include these by default, but if needed, on Debian/Ubuntu-based systems you can install them with:

```bash
sudo apt install build-essential
```

## Compiling and Running

🔧 **Compiling the Server**  
Use the following command to compile `server.c`:

```bash
gcc -o server server.c -lssl -lpthread -lcrypto
```

- -lssl: Links against the OpenSSL SSL library.
- -lcrypto: Links against the OpenSSL cryptographic library.
- -lpthread: Enables POSIX threading support.

🔧 **Compiling the Client**  
Use the following command to compile `client.c`:

```bash
gcc -o client client.c -lpthread
```
- -lpthread: Enables POSIX threading support for concurrent operations.

▶️ **Running the Server and Client**  
Once compiled:

Start the server (in one terminal):

```bash
./server
```

Start the client (in another terminal):

```bash
./client
```


## Usage

1. **Launching the Platform**:
   Run the compiled executable to start the platform.

2. **Adding Questions**:
   Follow the on-screen instructions to add new questions.

3. **Answering Questions**:
   Select a question and provide answers as prompted.

4. **Searching**:
   Use the search feature to look for specific questions or answers.

5. **Exiting**:
   Use the appropriate menu option to exit the platform safely.

## Features of `client.c`

The `client.c` file implements the client-side functionality for the **Question and Answer Platform**, enabling users to interact with the server using a menu-driven interface. Below are the key features:

### General Features
- **Socket Connection**: Establishes a TCP connection to the server using sockets.
- **Menu-Driven Interface**: Displays a dynamic menu based on the user's authentication status.

### User Authentication
- **Register**: Allows new users to register by sending their credentials to the server.
- **Login**: Authenticates existing users and provides access to additional features.

### Question and Answer Management
- **Post a Question**: Users can post new questions to the platform.
- **List Questions**: Retrieves and displays a list of all available questions with details (e.g., author, number of answers).
- **Answer a Question**: Enables users to submit answers to specific questions.
- **Search Questions**: Allows users to search for specific questions and view their details, including answers.

### Interactive Features
- **Rate an Answer**: Users can rate answers to a question on a scale of 1 to 5.
- **View Leaderboard**: Displays the leaderboard showing top contributors based on ratings or activity.

### Additional Functionalities
- **Dynamic Input Handling**: Processes user input for various operations.
- **Data Parsing and Display**: Parses server responses and displays data in a user-friendly format (e.g., questions, answers, leaderboard).
- **Error Handling**: Handles invalid inputs and server errors gracefully.
- **Exit**: Safely terminates the connection and exits the program.

This file is an essential part of the platform, providing a seamless interface for users to interact with the server and manage the Q&A system.


## Commands and Structures in `client.c`

The `client.c` file is the client-side implementation of the **Question and Answer Platform**. Below is a detailed explanation of its commands and structures:

---

### **Header Files**
- **`#include <stdio.h>`**  
  Provides functions for input and output operations such as `printf`, `scanf`, and `fgets`.

- **`#include <stdlib.h>`**  
  Provides utility functions like `exit()` and memory management functions.

- **`#include <string.h>`**  
  Used for string manipulation functions like `strcpy`, `strtok`, and `strcat`.

- **`#include <unistd.h>`**  
  Provides POSIX API functions such as `close()` for closing the socket connection.

- **`#include <arpa/inet.h>`**  
  Defines structures and functions for internet operations like `inet_pton` and `htons`.

- **`#include <sys/socket.h>`**  
  Used to create and manage sockets for communication.

---

### **Macros**
- **`#define PORT 8080`**  
  Specifies the port number used to connect to the server.

- **`#define BUFFER_SIZE 2048`**  
  Defines the size of the buffer used for sending and receiving data between the client and server.

---

### **Functions**

#### **`print_menu(int authenticated)`**
Displays a menu based on the user's authentication status:
- **Unauthenticated Users**: Options for registration and login.
- **Authenticated Users**: Options to post questions, answer questions, search, rate answers, view leaderboard, and exit.

#### **`display_questions(const char *buffer)`**
Parses and displays a list of questions retrieved from the server:
1. Skips the response status (e.g., `OK|`) in the server's response.
2. Tokenizes the data payload using `strtok` with `;` as the delimiter.
3. Extracts question details (ID, question text, author, and answer count).
4. Prints the parsed questions in a user-friendly format.

#### **`display_search_results(const char *buffer)`**
Displays search results for a specific query:
1. Parses the response into distinct parts:
   - Status (`OK` or `ERROR`).
   - Question text or "Question not found".
   - Answers or "No answers yet".
2. If the question is found, tokenizes and displays the answers.

---

### **Socket and Communication**

#### **Socket Creation**
- **`int sock = socket(AF_INET, SOCK_STREAM, 0);`**  
  Creates a TCP socket:
  - `AF_INET`: Specifies IPv4 addressing.
  - `SOCK_STREAM`: Indicates a TCP socket.

#### **Server Address Setup**
- **`struct sockaddr_in serv_addr;`**  
  A structure holding the server's address details:
  - `sin_family`: Specifies the address family (IPv4).
  - `sin_port`: Sets the port number using `htons(PORT)`.
  - `sin_addr`: Converts the IP address from text to binary using `inet_pton`.

#### **Connection Establishment**
- **`connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));`**  
  Establishes a connection to the server using the socket.

---

### **Menu Options**

#### **1. Register**
- Prompts the user for a username and password.
- Sends the formatted command `REGISTER|username|password` to the server.
- Displays the server's response.

#### **2. Login**
- Prompts the user for a username and password.
- Sends the formatted command `LOGIN|username|password` to the server.
- Parses the server's response to determine if login is successful.

#### **3. Post a Question**
- Prompts the user to enter a question.
- Sends the formatted command `POST|question` to the server.
- Displays the server's response.

#### **4. List Questions**
- Sends the `LISTQ` command to the server.
- Parses and displays the list of questions using `display_questions`.

#### **5. Answer a Question**
- Prompts the user for a question number and their answer.
- Sends the formatted command `ANSWER|question_number|answer` to the server.
- Displays the server's response.

#### **6. Search Questions**
- Prompts the user for a search query.
- Sends the formatted command `SEARCH|query` to the server.
- Parses and displays the search results using `display_search_results`.

#### **7. Rate an Answer**
- Prompts the user for the question ID, answer number, and rating.
- Sends the formatted command `RATE|question_id|answer_number|rating` to the server.
- Displays the server's response.

#### **8. View Leaderboard**
- Sends the `LEADER` command to the server.
- Parses and displays the leaderboard.

#### **9. Exit**
- Closes the socket connection using `close(sock)`.
- Exits the program using `exit(0)`.

---

### **String and Buffer Handling**

#### **String Manipulation**
- **`snprintf`**: Safely formats strings to construct commands like `REGISTER`, `LOGIN`, `POST`, etc.
- **`strtok`**: Splits strings into tokens based on a delimiter. Used to parse server responses.
- **`strchr`**: Finds the first occurrence of a character in a string. Used to skip headers in server responses.

#### **Buffer Management**
- **`char buffer[BUFFER_SIZE] = {0};`**: Initializes the buffer with zeros to avoid garbage data.
- **`memset(buffer, 0, BUFFER_SIZE);`**: Clears the buffer after each interaction.

---

### **Network Communication**

#### **Sending Data**
- **`send(sock, buffer, strlen(buffer), 0);`**  
  Sends data from the client to the server.

#### **Receiving Data**
- **`recv(sock, buffer, BUFFER_SIZE, 0);`**  
  Receives data from the server into the buffer.

---

### **Error Handling**
- Checks for invalid inputs and server errors.
- Displays appropriate error messages if the connection fails or an invalid menu option is selected.

---

This detailed explanation covers all commands and structures used in `client.c`. Each part of the code is designed to ensure smooth communication with the server and provide an interactive experience for the user.


## Features of `server.c`

The `server.c` file implements the server-side functionality for the **Question and Answer Platform**, enabling multiple clients to connect and interact with the system concurrently. Below are the key features:

---

### **General Features**
- **Multi-Client Support**: Handles up to `MAX_CLIENTS` simultaneously using multithreading (`pthread` library).
- **Secure Password Storage**: Uses SHA-256 hashing for securely storing user passwords.
- **Persistent Data Storage**: Saves user and question data to local files (`users.dat` and `questions.dat`) and loads them on startup.
- **Command-Based Interaction**: Processes client commands such as registration, login, posting questions, answering, searching, and more.

---

### **User Management**
- **Register**: Allows new users to register by providing a username and password. Initial credits are set to 100.
- **Login**: Authenticates users based on their username and password.
- **Logout**: Logs a user out of the system.
- **Persistent User Data**: Saves user data (e.g., username, password hash, credits, scores) securely to disk.

---

### **Question Management**
- **Post Questions**: Allows authenticated users to post questions. Users earn 10 credits for each question posted.
- **List Questions**: Retrieves a list of all questions with details like author and the number of answers.
- **Answer Questions**: Enables users to submit answers to specific questions. Users earn 5 credits for each answer submitted.
- **Search Questions**: Searches for questions based on a keyword and displays matching results, including answers.
- **Rate Answers**: Allows question authors to rate answers on a scale, rewarding answer authors with points.

---

### **Leaderboard**
- **View Leaderboard**: Displays the top 10 users based on their cumulative scores, rewarding active contributors.

---

### **Data Persistence**
- **Load Data**:
  - Loads user and question data from disk files (`users.dat`, `questions.dat`) on startup.
- **Save Data**:
  - Saves user and question data to disk after every update, ensuring persistence across server restarts.

---

### **Security**
- **Password Hashing**: Uses the SHA-256 algorithm for hashing passwords before storing them.
- **Session Management**: Tracks the authentication state of each client session to ensure secure access to features.

---

### **Networking**
- **Socket Programming**: Implements a TCP server using sockets to handle communication with clients.
- **Port Configuration**: Listens for incoming client connections on port `8080`.
- **Threaded Client Handling**: Spawns a new thread for each connected client, ensuring concurrent processing of requests.

---

### **Error Handling**
- Validates inputs and handles errors gracefully, such as:
  - Duplicate usernames during registration.
  - Invalid login credentials.
  - Exceeding question or answer limits.
  - Unauthorized actions (e.g., rating answers by non-authors).

---

### **Technical Details**
- **Thread Safety**: Protects shared resources (e.g., user and question arrays) with mutex locks to prevent race conditions.
- **Custom Data Structures**:
  - **User**: Stores user details such as username, password hash, credits, and scores.
  - **Question**: Stores question details, including answers and ratings.
  - **ClientSession**: Maintains session-specific details like the socket descriptor and authentication state.

---

This implementation forms the backbone of the **Question and Answer Platform**, providing robust, secure, and scalable server-side functionality.


## Commands and Structures in `server.c`

The `server.c` file implements the server-side functionality of the **Question and Answer Platform**, handling multiple clients concurrently, managing user data, and processing commands.

---

### **Header Files**
- **`#include <stdio.h>`**  
  Provides functions for input and output operations, such as `printf`, `snprintf`, and file handling (`fopen`, `fread`, `fwrite`).

- **`#include <stdlib.h>`**  
  Provides utility functions like memory allocation (`malloc`, `free`) and program termination (`exit`).

- **`#include <string.h>`**  
  Includes string manipulation functions like `strcpy`, `strncpy`, `strcat`, `strcmp`, and `strtok`.

- **`#include <pthread.h>`**  
  Provides threading capabilities to handle multiple client connections concurrently.

- **`#include <unistd.h>`**  
  Includes POSIX API functions like `close` for closing sockets and `sleep`.

- **`#include <arpa/inet.h>`**  
  Provides functions for network communication, such as `htons`, `inet_pton`, and socket-related structures.

- **`#include <sys/socket.h>`**  
  Enables socket programming and includes definitions for creating and managing sockets.

- **`#include <openssl/sha.h>`**  
  Provides functions for hashing (e.g., `SHA256`), used for securely storing user passwords.

---

### **Macros**
- **`#define PORT 8080`**  
  Specifies the server's listening port for incoming client connections.

- **`#define MAX_CLIENTS 10`**  
  Sets the maximum number of concurrent client connections.

- **`#define BUFFER_SIZE 2048`**  
  Defines the size of the buffer for communication between the server and clients.

- **`#define MAX_USERS 100`**  
  Limits the number of registered users.

- **`#define MAX_QUESTIONS 1000`**  
  Limits the number of stored questions.

- **`#define MAX_ANSWERS 20`**  
  Limits the number of answers per question.

---

### **Structures**

#### **`User`**
Stores user details:
- **`char username[50]`**: The username of the user.
- **`char password_hash[SHA256_DIGEST_LENGTH*2 + 1]`**: The hashed password stored as a hex string.
- **`int credits`**: The user's credits (used for posting questions and answers).
- **`int is_manager`**: Reserved for future admin functionality.
- **`int score`**: The cumulative score from rated answers.

#### **`Question`**
Stores question details:
- **`char question[256]`**: The question text.
- **`char author[50]`**: The username of the question's author.
- **`char answers[MAX_ANSWERS][256]`**: Array to store answers to the question.
- **`char answer_authors[MAX_ANSWERS][50]`**: Array to store usernames of the answer authors.
- **`int answer_count`**: The number of answers provided for the question.
- **`int ratings[MAX_ANSWERS]`**: Array to store ratings for each answer.

#### **`ClientSession`**
Stores session-specific details for each connected client:
- **`int sock`**: The socket descriptor for the client's connection.
- **`struct sockaddr_in addr`**: The client's address information.
- **`int user_idx`**: The index of the authenticated user in the `users` array.
- **`int authenticated`**: Indicates whether the client is logged in (1) or not (0).

---

### **Global Variables**
- **`User users[MAX_USERS]`**: Array to store user records.
- **`Question questions[MAX_QUESTIONS]`**: Array to store questions and their answers.
- **`int user_count`**: Keeps track of the number of registered users.
- **`int question_count`**: Keeps track of the number of posted questions.

- **`pthread_mutex_t users_mutex`**: Mutex lock for synchronizing access to the `users` array.
- **`pthread_mutex_t questions_mutex`**: Mutex lock for synchronizing access to the `questions` array.

---

### **Functions**

#### **Hashing and Data Persistence**
1. **`void hash_password(const char *password, char *output)`**  
   - Hashes a plaintext password using SHA-256 and converts it into a hex string.

2. **`void load_data()`**  
   - Loads user and question data from files (`users.dat` and `questions.dat`) into memory.

3. **`void save_users()`**  
   - Saves the `users` array to the `users.dat` file.

4. **`void save_questions()`**  
   - Saves the `questions` array to the `questions.dat` file.

---

#### **User Management**
1. **`int find_user(const char *username)`**  
   - Searches for a user by username and returns their index in the `users` array. Returns `-1` if not found.

2. **`void handle_register(int sock, char *username, char *password)`**  
   - Handles the `REGISTER` command:
     - Checks for duplicate usernames.
     - Hashes the password and stores the new user in the `users` array.
     - Grants 100 initial credits to the user.

3. **`void handle_login(ClientSession *session, char *username, char *password)`**  
   - Handles the `LOGIN` command:
     - Authenticates the user by comparing the hashed password.
     - Updates the client session with the authenticated user's details.

4. **`void handle_logout(ClientSession *session)`**  
   - Handles the `LOGOUT` command:
     - Resets the client's session to unauthenticated.

---

#### **Question Management**
1. **`void handle_post_question(ClientSession *session, char *question_text)`**  
   - Handles the `POST` command:
     - Adds a new question to the `questions` array.
     - Rewards the user with 10 credits for posting.

2. **`void handle_list_questions(ClientSession *session)`**  
   - Handles the `LISTQ` command:
     - Sends a list of all questions to the client, including their index, author, and answer count.

3. **`void handle_answer(ClientSession *session, char *qidx_str, char *answer_text)`**  
   - Handles the `ANSWER` command:
     - Adds an answer to the specified question.
     - Rewards the user with 5 credits for answering.

4. **`void handle_search(ClientSession *session, char *keyword)`**  
   - Handles the `SEARCH` command:
     - Searches for a question by keyword and sends the matching question and its answers to the client.

5. **`void handle_rate_answer(ClientSession *session, char *qidx_str, char *aidx_str, char *score_str)`**  
   - Handles the `RATE` command:
     - Allows the question's author to rate an answer.
     - Updates the answer author's score with the given rating.

---

#### **Leaderboard**
1. **`void handle_leaderboard(ClientSession *session)`**  
   - Handles the `LEADER` command:
     - Sorts users by their scores in descending order.
     - Sends the top 10 users and their scores to the client.

---

#### **Networking**
1. **`void *client_handler(void *arg)`**  
   - Handles a single client's commands in a separate thread.
   - Processes commands like `REGISTER`, `LOGIN`, `POST`, `ANSWER`, `RATE`, and more.

2. **`int main()`**  
   - Entry point for the server:
     - Loads saved data.
     - Sets up the server socket.
     - Listens for incoming client connections.
     - Spawns a new thread for each connected client.

---

### **Key Commands**
The server processes the following commands sent by clients:
1. **`REGISTER|username|password`**: Registers a new user.
2. **`LOGIN|username|password`**: Logs in an existing user.
3. **`LOGOUT`**: Logs out the current user.
4. **`POST|question_text`**: Posts a new question.
5. **`LISTQ`**: Lists all questions.
6. **`ANSWER|question_index|answer_text`**: Answers a specific question.
7. **`SEARCH|keyword`**: Searches for questions by keyword.
8. **`RATE|question_index|answer_index|score`**: Rates an answer.
9. **`LEADER`**: Displays the leaderboard.

---

### **Thread Safety**
- All shared resources (e.g., `users` and `questions` arrays) are protected with mutex locks to prevent race conditions in a multithreaded environment.

---

## Working of the Client Side Process

1. Registration of the user:
   <br><img src="Screenshots/register.png" alt="Alt text" width="500"/>
2. User logging in:
   <br><img src="Screenshots/login.png" alt="Alt text" width="500"/>
3. Question posting feature:
   <br><img src="Screenshots/post.png" alt="Alt text" width="500"/>
4. Question listing feature:
   <br><img src="Screenshots/list.png" alt="Alt text" width="500"/>
5. Question answering feature:
   <br><img src="Screenshots/answer.png" alt="Alt text" width="500"/>
6. Question searching feature:
   <br><img src="Screenshots/search.png" alt="Alt text" width="500"/>
7. Answer rating feature:
   <br><img src="Screenshots/rating.png" alt="Alt text" width="500"/>
8. Leaderboard feature:
   <br><img src="Screenshots/leaderboard.png" alt="Alt text" width="500"/>
9. Exit:
   <br><img src="Screenshots/exit.png" alt="Alt text" width="500"/>


## Project Responsibility

| Section | Responsibility          | Team Members                                   |
|---------|-------------------------|------------------------------------------------|
| A       | **System Design**       | Devansh Srivastava, Pranjal Singh              |
| B       | **Project Planning**    | Sagar Bejawada, Ritesh Kumar Yadav             |
| C       | **Coding**              | Aayush Anand, Nirvik Jana, Devansh Srivastava  |
| D       | **PPT Design**          | Subhamita Halder, Shaswat Ranjan               |
| E       | **UI Interfacing**      | Aditya Pratap Singh, Syamantak Banerjee        |
| F       | **Resource Management** | Kunal Singh Munda, Ritesh Kumar Yadav          |
