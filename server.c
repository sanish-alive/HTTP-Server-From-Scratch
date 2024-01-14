#include <winsock2.h>
#include <stdio.h>

#define DEFAULT_PORT 5000
#define DEFAULT_BUFLEN 1024

char* readFile(const char* filename, long* fileSize);

int main(int argc, char **argv) {
    printf("Http Server From Scratch");

    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    char recvBuffer[DEFAULT_BUFLEN];
    int recvResult;
    int sendResult;

    char* fileContent = NULL , *httpResponse = NULL;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Createing a socket to listen for incoming connections
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(DEFAULT_PORT);

    // Binding the socket
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", DEFAULT_PORT);

    while (1) {
        //Accept a client connection
        clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if(clientSocket == INVALID_SOCKET) {
            printf("Accept failed\n");
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // Receive and print the HTTP request
        recvResult = recv(clientSocket, recvBuffer, DEFAULT_BUFLEN, 0);
        if (recvResult > 0) {
            printf("Received HTTP request:\n%s\n", recvBuffer);
            
            // response file handling
            const char* filename = "index.html";
            long fileSize;
            fileContent = readFile(filename, &fileSize);
            if (fileContent == NULL) {
                printf("File Error: %s\n", filename);
                continue;
            }

            // Response content for client
            const char* httpResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            httpResponse = (char*)malloc(strlen(httpResponseHeader) + fileSize + 1);
            if (httpResponse == NULL) {
                printf("Memory allocation failed\n");
                free(fileContent);
                closesocket(clientSocket);
                continue;
            }

            // Copy the header and file content into the response buffer
            strcpy(httpResponse, httpResponseHeader);
            strcat(httpResponse, fileContent);


            sendResult = send(clientSocket, httpResponse, strlen(httpResponse), 0);
            if (sendResult == SOCKET_ERROR) {
                printf("Send failed\n");
            }
        } else if (recvResult == 0){
            printf("Connection closed by client\n");
        } else {
            printf("Recv failed\n");
        }

        // Close the client socket
        free(fileContent);
        free(httpResponse);
        closesocket(clientSocket);
    }

    // Clean up Winsock
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

char* readFile(const char* filename, long* fileSize) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to store the file content
    char* fileContent = (char*)malloc(*fileSize + 1);
    if (fileContent == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the file content into buffer
    fread(fileContent, 1, *fileSize, file);
    fclose(file);

    fileContent[*fileSize] = '\0';

    return fileContent;
}