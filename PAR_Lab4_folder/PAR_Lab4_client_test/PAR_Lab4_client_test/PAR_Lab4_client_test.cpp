#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void sendCommand(SOCKET sock, const string& command) 
{
    string toSend = command + "\n";
    send(sock, toSend.c_str(), toSend.size(), 0);

    char buffer[1024];
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) 
    {
        buffer[bytesReceived] = '\0';
        string response(buffer);
        
        size_t pos = 0;
        while ((pos = response.find('\n')) != string::npos) 
        {
            string line = response.substr(0, pos);
            cout << "Server response to '" << command << "': " << line << endl;
            response.erase(0, pos + 1);
        }
    } else 
    {
        cout << "Error receiving response." << endl;
    }
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        cerr << "Connect failed." << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    sendCommand(sock, "STATUS");

    sendCommand(sock, "FOO_BAR");

    sendCommand(sock, "INIT 10 2");

    sendCommand(sock, "START");

    sendCommand(sock, "START");

    sendCommand(sock, "STATUS");

    this_thread::sleep_for(chrono::seconds(1));

    sendCommand(sock, "STATUS");

    closesocket(sock);
    WSACleanup();
    return 0;
}
