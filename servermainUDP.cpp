#include "servermain.h"
using namespace std;

# define BUFFER_SIZE 1024

void commicateWithServer(const char* serverName, const char* serverIP, int serverPort){
    int serverSocket;
    char buffer[BUFFER_SIZE];
    sockaddr_in serverAddress;

    // create socket by using UDP
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1){
        cerr << "Error creating socket\n";
        exit(1);
    }

    // configure server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddress.sin_addr);

    // send message to the specific server
    string message = "Hello Server";
    message += serverName;
    if (sendto(serverSocket, message.c_str(), message.size(), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        cerr << "Failed to send message to Server " << serverName << endl;
        close(serverSocket);
        exit(1); 
    }
    cout << "Message sent to Server " << serverName << endl;

    // receive response from the server
    socklen_t length = sizeof(serverAddress);
    int bytes = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&serverAddress, &length);
    if (bytes < 0){
        cerr << "Failed to receive response from Server " << serverName << endl;
    }
    else{
        buffer[bytes] = '\0';
        cout << "Received from Server " << serverName << ": " << buffer << endl;
    }

    // close
    close(serverSocket);
    exit(0);
}

int main(){
    // Server infomation
    struct ServerInfo{
        const char* name;
        const char* ip;
        int port;
    } servers[] = {
        {"A", "127.0.0.1", 30778},
        {"B", "127.0.0.1", 31778},
        {"C", "127.0.0.1", 32778}
    };

    // create child process for each server connection
    for (int i = 0; i < 3; ++i){
        pid_t pid = fork();
        if (pid < 0){
            cerr << "Failed to fork for Server " << servers[i].name << endl;
        }
        else if (pid == 0){
            commicateWithServer(servers[i].name, servers[i].ip, servers[i].port);
        }
    }

    // wait for all child process to complete
    for (int i = 0; i < 3; ++i){
        wait(NULL);
    }
    
    return 0;
}
