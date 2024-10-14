#include "client.h"
using namespace std;

int main(){
    // define
    int clientSocket;
    sockaddr_in serverAddress;
    const int PORT = 24778;

    // creating 
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error creating socket\n";
        return 1;
    }

    // specify
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // sending request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        cerr << "Connection failed\n";
        return 1;
    }

    cout << "Connected to server\n";

    // sending data
    string str;
    cout << "-----Start a new query-----" << endl;
    cout << "Enter Department Name: ";
    cin >> str;
    cout << endl;

    const char* message = str.c_str();
    if (send(clientSocket, message, strlen(message), 0) < 0){
        cerr << "Send failed\n";
        return 1;
    }

    cout << "Message sent\n";

    // Receiving the reponse from the server
    char buffer[1024] = { 0 };
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0){
        cerr << "Failed to receive response from server\n";
    } else{
        cout << buffer << endl;
    }

    // closing socket
    close(clientSocket);

    return 0;
}