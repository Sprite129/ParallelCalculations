#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define ROOT_DIR "www"

void sendResponse(SOCKET clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.size(), 0);
}

std::string readFileContent(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

void handleRequest(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::string request(buffer);

        std::cout << "Received request:\n" << request << std::endl;

        size_t pos = request.find(" ");
        size_t pos2 = request.find(" ", pos + 1);
        std::string method = request.substr(0, pos);
        std::string path = request.substr(pos + 1, pos2 - pos - 1);

        if (method != "GET") {
            closesocket(clientSocket);
            return;
        }

        if (path == "/") {
            path = "/index.html";
        }

        std::string fullPath = std::string(ROOT_DIR) + path;
        std::string content = readFileContent(fullPath);

        std::string response;
        if (content.empty()) {
            std::string notFound = "<html><body><h1>404 Not Found</h1></body></html>";
            response = "HTTP/1.1 404 Not Found\r\nContent-Length: " + std::to_string(notFound.length()) +
                "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n" + notFound;
        }
        else {
            response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(content.length()) +
                "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n" + content;
        }

        sendResponse(clientSocket, response);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "Connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << "\n";

        std::thread clientThread(handleRequest, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
