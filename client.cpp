#include "client.h"
using namespace std;

const int PORT = 24778;

int main(){
    // define
    int clientSocket;
    sockaddr_in serverAddress;
    socklen_t addressLength = sizeof(serverAddress);

    // creating 
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating socket\n";
        return 1;
    }

    // specify
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // sending request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        cerr << "Connection failed\n";
        return 1;
    }

    // Get the port number assigned by the OS
    if (getsockname(clientSocket, (struct sockaddr *)&serverAddress, &addressLength) == -1) {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    int dynamicPort = ntohs(serverAddress.sin_port);

    cout << "Client is up and running\n";

    while (true) {
        // sending data
        string str;
        cout << "Enter Department Name: ";
        cin >> str;
        cout << endl;

        if (send(clientSocket, str.c_str(), strlen(str.c_str()), 0) < 0){
            cerr << "Send failed\n";
            return 1;
        }

        cout << "Client has sent Department " << str << " to Main Server using TCP over port "
            << dynamicPort << '.' << endl;

        // Receiving the reponse from the server
        char buffer[1024] = { 0 }; // clear buffer before each new recv 
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0){
            cerr << "Failed to receive response from server\n";
        } else{
            cout << buffer << endl;
        }
        cout << "-----Start a new query-----" << endl;
    }
    // closing socket
    close(clientSocket);

    return 0;
}