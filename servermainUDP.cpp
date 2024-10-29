#include "servermain.h"
using namespace std;

int main(){
    const int PORT = 24778;
    const int BUFFER_SIZE = 1024;

    int len;
    int serverSocket;
    char buffer[BUFFER_SIZE];
    sockaddr_in serverAddress, clientAddress;
    socklen_t length = sizeof(clientAddress);

    // create socket by using UDP
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1){
        cerr << "Error creating socket\n";
        return -1;
    }

    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    
    // build the socket
    if (::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        cerr << "Bind failed";
        return -1;
    }

    // receive incoming datagrams from clients
    while (true){
        len = recvfrom(serverSocket, (char*)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&clientAddress, &length);
        buffer[len] = '\0';
        cout << "Client: " << buffer << endl;

        string response = "Hello from server";
        sendto(serverSocket, response.c_str(), response.size(), 0, (const struct sockaddr *)&clientAddress, length);
        cout << "Response sent to client." << endl;
    }
    
    close(serverSocket);
    return 0;
}
