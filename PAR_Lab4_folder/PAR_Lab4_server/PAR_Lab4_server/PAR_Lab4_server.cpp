#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct ClientSession 
{
    SOCKET socket;
    int size = 0;
    int threadsNum = 0;
    vector<vector<int>> matrix;
    bool started = false;
    bool finished = false;
    double elapsedTime = 0.0;
    mutex mtx;
};

void generateMatrix(vector<vector<int>>& matrix, int begin, int end, int size) 
{
    for (int i = begin; i < end; i++) 
    {
        int n = i / size;
        int m = i % size;
        matrix[n][m] = rand() % 200 - 100;
    }
}

void diagonalMin(vector<vector<int>>& matrix, int begin, int end, int size) 
{
    for (int i = begin; i < end; i++) 
    {
        int n = i / size;
        int minVal = 200;
        for (int m = 0; m < size; m++) 
        {
            if (matrix[n][m] < minVal) 
            {
                minVal = matrix[n][m];
            }
        }
        matrix[n][size - n - 1] = minVal;
    }
}

void processMatrix(ClientSession& session) 
{
    vector<thread> threads;
    int elements = session.size * session.size;
    int chunkSize = (elements + session.threadsNum - 1) / session.threadsNum;

    auto startTime = chrono::high_resolution_clock::now();

    for (int i = 0; i < session.threadsNum; i++) 
    {
        int begin = i * chunkSize;
        int end = min(begin + chunkSize, elements);
        threads.emplace_back(generateMatrix, ref(session.matrix), begin, end, session.size);
    }
    for (auto& t : threads) t.join();
    threads.clear();

    for (int i = 0; i < session.threadsNum; i++) 
    {
        int begin = i * chunkSize;
        int end = min(begin + chunkSize, elements);
        threads.emplace_back(diagonalMin, ref(session.matrix), begin, end, session.size);
    }
    for (auto& t : threads) t.join();

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = endTime - startTime;

    session.elapsedTime = elapsed.count();
}

void handleClient(ClientSession& session) 
{
    char buffer[1024];

    while (true) 
    {
        int bytesReceived = recv(session.socket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) 
        {
            cout << "Client disconnected or recieved error." << endl;
            break;
        }
        buffer[bytesReceived] = '\0';

        string command(buffer);
        command.erase(command.find_last_not_of("\r\n") + 1);

        cout << "Received: " << command << endl;

        if (command.rfind("INIT", 0) == 0) 
        {
            sscanf_s(buffer, "INIT %d %d", &session.size, &session.threadsNum);
            session.matrix = vector<vector<int>>(session.size, vector<int>(session.size, 0));
            send(session.socket, "OK\n", 3, 0);
        }
        else if (command == "START") 
        {
            lock_guard<mutex> lock(session.mtx);
            if (!session.started) 
            {
                session.started = true;
                session.finished = false;
                thread([&session]() {
                    processMatrix(session);
                    lock_guard<mutex> lock(session.mtx);
                    session.finished = true;
                    }).detach();
                    send(session.socket, "STARTED\n", 8, 0);
            }
            else 
            {
                send(session.socket, "ERROR! Already started\n", 23, 0);
            }
        }
        else if (command == "STATUS") 
        {
            lock_guard<mutex> lock(session.mtx);
            if (!session.started) 
            {
                send(session.socket, "ERROR! Not started\n", 19, 0);
            }
            else if (!session.finished)
            {
                send(session.socket, "WORKING...\n", 12, 0);
            }
            else 
            {
                char response[64];
                sprintf_s(response, sizeof(response), "FINISHED %.4f\n", session.elapsedTime);
                send(session.socket, response, strlen(response), 0);
            }
        }
        else if (command == "SHUTDOWN") 
        {
            send(session.socket, "BYE\n", 4, 0);
            cout << "Client requested shutdown." << endl;
            break;
        }
        else 
        {
            send(session.socket, "ERROR! Unknown command\n", 23, 0);
        }
    }

    closesocket(session.socket);
}

int main() 
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) 
    {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        cerr << "Bind failed." << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        cerr << "Listen failed." << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server started. Waiting for clients..." << endl;

    while (true) 
    {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) 
        {
            cerr << "Accept failed." << endl;
            continue;
        }

        cout << "Client connected." << endl;
        auto session = make_shared<ClientSession>();
        session->socket = clientSocket;
        thread([session]() { handleClient(*session); }).detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
