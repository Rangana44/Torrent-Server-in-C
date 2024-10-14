 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT 2236
#define BACKLOG 5
#define FILE_DIR "/home/maleesha/Documents/send1"
#define LOG_FILE "log_srv5200"

void log_event(const char *client_ip, int client_port, const char *event, const char *filename, int success) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now;
    struct tm *time_info;
    char time_str[26];

    time(&now);
    time_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    fprintf(log_file, "[%s] Client: %s:%d, Event: %s, File: %s, Status: %s\n",
            time_str, client_ip, client_port, event, filename, success ? "Success" : "Fail");

    fclose(log_file);
}

void listFiles(const char *path, char *fileList, char files[][256], int *fileCount) {
    struct dirent *dp;
    DIR *dir = opendir(path);
    *fileCount = 0;

    if (dir == NULL) {
        strcpy(fileList, "Could not open directory");
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_type == DT_REG) { // Only list regular files
            sprintf(fileList + strlen(fileList), "%d. %s\n", *fileCount + 1, dp->d_name);
            strcpy(files[*fileCount], dp->d_name);
            (*fileCount)++;
        }
    }

    closedir(dir);
}

void sendFile(int socket, const char *filename, const char *client_ip, int client_port) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILE_DIR, filename);

    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("File open failed");
        log_event(client_ip, client_port, "File Transfer", filename, 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char buffer[1024];
    size_t bytesRead;
    long totalBytesSent = 0;

    send(socket, &fileSize, sizeof(fileSize), 0);

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(socket, buffer, bytesRead, 0) == -1) {
            perror("File send failed");
            fclose(file);
            log_event(client_ip, client_port, "File Transfer", filename, 0);
            return;
        }
        totalBytesSent += bytesRead;
    }

    fclose(file);
    log_event(client_ip, client_port, "File Transfer", filename, 1);
}

void handle_client(int new_socket, const char *client_ip, int client_port) {
    char buffer[1024] = {0};
    char fileList[2048] = {0};
    char files[100][256];
    int fileCount = 0;

    listFiles(FILE_DIR, fileList, files, &fileCount);

    send(new_socket, fileList, strlen(fileList), 0);
    printf("File list sent to %s:%d\n", client_ip, client_port);

    read(new_socket, buffer, 1024);
    int fileIndex;
    sscanf(buffer, "%d", &fileIndex);

    if (fileIndex > 0 && fileIndex <= fileCount) {
        sendFile(new_socket, files[fileIndex - 1], client_ip, client_port);
        printf("File %s sent to %s:%d\n", files[fileIndex - 1], client_ip, client_port);
    } else {
        char *errorMsg = "Invalid file number";
        send(new_socket, errorMsg, strlen(errorMsg), 0);
        printf("Invalid file number received from %s:%d\n", client_ip, client_port);
        log_event(client_ip, client_port, "Invalid File Request", "N/A", 0);
    }

    close(new_socket);
}

void sigchld_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pid_t pid;

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(address.sin_port);

        printf("New connection from %s:%d\n", client_ip, client_port);
        log_event(client_ip, client_port, "Connection", "N/A", 1);

        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            close(server_fd);
            handle_client(new_socket, client_ip, client_port);
            exit(0);
        } else {
            close(new_socket);
        }
    }

    return 0;
}