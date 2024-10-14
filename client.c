#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 2236  // Change to match the server's port
#define SAVE_DIR "/home/maleesha/Documents/receive"  // Directory to save received files

void receiveFile(int socket, const char *filename) {
    char filepath[512];
    sprintf(filepath, "%s/%s", SAVE_DIR, filename);  // Construct the full path

    FILE *file = fopen(filepath, "wb");
    if (file == NULL) {
        perror("File open failed");
        return;
    }

    long fileSize;
    read(socket, &fileSize, sizeof(fileSize));

    char buffer[1024];
    ssize_t bytesRead;
    long totalBytesReceived = 0;
    int progress = 0;

    // Receiving the file in chunks and writing to the file
    while ((bytesRead = read(socket, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytesRead, file);
        totalBytesReceived += bytesRead;
        int newProgress = (totalBytesReceived * 100) / fileSize;
        if (newProgress > progress) {
            progress = newProgress;
            printf("Progress: %d%%\n", progress);
        }
    }

    fclose(file);
}

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Creating the directory if it doesn't exist
    struct stat st = {0};
    if (stat(SAVE_DIR, &st) == -1) {
        mkdir(SAVE_DIR, 0700);  // Creating the directory with read/write/execute permissions
    }

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);  // Match the server's port

    // Converting IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    printf("Connecting to server at 127.0.0.1:%d...\n", PORT);

    // Connecting to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("\nConnection Failed");
        return -1;
    }

    printf("Connected to the server.\n");

    // Reading the list of files from the server
    valread = read(sock, buffer, 1024);
    printf("Available files:\n%s\n", buffer);

    // Prompting the user for the file number and file name
    printf("Enter the file number to download: ");
    int fileNumber;
    scanf("%d", &fileNumber);
    printf("Enter the file name to save as: ");
    char fileName[256];
    scanf("%s", fileName);

    // Sending the file number and file name to the server
    sprintf(buffer, "%d %s", fileNumber, fileName);
    send(sock, buffer, strlen(buffer), 0);

    printf("Downloading %s...\n", fileName);

    // Receiving the requested file from the server and saving to the specified directory
    receiveFile(sock, fileName);

    printf("Download complete. File saved in directory %s as %s\n", SAVE_DIR, fileName);

    return 0;
}