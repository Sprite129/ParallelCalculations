#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

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
        
        cout << "Server response to '" << command << "': " << response;
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
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

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

    Sleep(1000);

    sendCommand(sock, "STATUS");

    closesocket(sock);
    WSACleanup();
    return 0;
}
