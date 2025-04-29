#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void sendAndPrint(SOCKET sock, const string& message) 
{
    send(sock, message.c_str(), message.length(), 0);
    char buffer[1024];
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) 
    {
        buffer[bytesReceived] = '\0';
        cout << "Server response: " << buffer;
    }
}

int main() 
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) 
    {
        cerr << "Connection failed" << endl;
        return 1;
    }

    cout << "Connected to server" << endl;

    int matrixSize, threadsNum;
    cout << "Enter matrix size: ";
    cin >> matrixSize;
    cout << "Enter number of threads: ";
    cin >> threadsNum;

    sendAndPrint(sock, "INIT " + to_string(matrixSize) + " " + to_string(threadsNum) + "\n");
    sendAndPrint(sock, "START\n");

    while (true) 
    {
        cout << "Checking status..." << endl;
        send(sock, "STATUS\n", 7, 0);

        char buffer[1024];
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) 
        {
            buffer[bytesReceived] = '\0';
            cout << "Server response: " << buffer;

            if (string(buffer).find("FINISHED") == 0) 
            {
                break;
            }
        }
        Sleep(1000);
    }

    sendAndPrint(sock, "SHUTDOWN\n");

    closesocket(sock);
    WSACleanup();
    return 0;
}
