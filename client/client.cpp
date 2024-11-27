#include "client.h"
using namespace std;

#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class Client{
private:
    // encrption function
    string encrypt(const string& input){
        string encrypted = input;

        for (char& c: encrypted){
            if (c >= 'A' && c <= 'Z'){
                c = 'A' + (c - 'A' + 3) % 26;
            }
            else if (c >= 'a' && c <= 'z'){
                c = 'a' + (c - 'a' + 3) % 26;
            }
            else if (c >= '0' && c <= '9'){
                c = '0' + (c - '0' + 3) % 26;
            }
        }
        
        return encrypted;
    }

public:
    void start(){
        int clientSocket;
        struct sockaddr_in serverAddress;
        socklen_t addressLength = sizeof(serverAddress);
        char buffer[BUFFER_SIZE];

        // creatie socket by using TCP
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            cerr << "socket creation failed" << endl;
            exit(1);
        }

        // configure server address
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(MAIN_SERVER_TCP_PORT);
        inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

        // send connect request
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
            cerr << "connection failed" << endl;
            exit(1);
        }

        // get the port number assigned by the OS
        if (getsockname(clientSocket, (struct sockaddr *)&serverAddress, &addressLength) == -1) {
            perror("getsockname failed");
            exit(EXIT_FAILURE);
        }
        int dynamicPort = ntohs(serverAddress.sin_port);

        cout << "Clint is up and running" << endl;
        while (true){
            string username, password, department;
            string encryptedUsername, encryptedPassword;
            string message, dormitoryType;
            
            // input username
            cout << "Enter user name: ";
            cin >> username;
            // letter number constraint
            if (username.size() < 5 || username.size() > 50){
                cerr << "Error: Username must be between 5 and 50 lowercase characters." << endl;
                continue;
            }
            // lower letter constraint
            for (const char& c : username){
                if (!islower(c)){
                    cerr << "Error: Username must contain only lowercase characters." << endl;
                    continue;
                }
            }

            // input password
            cout << "Enter password: ";
            cin >> password;
            // letter number constraint
            if (password.size() < 5 || password.size() > 50){
                cerr << "Error: Password must be between 5 and 50 lowercase characters." << endl;
                continue;
            }

            // input department name
            cout << "Enter department name: ";
            cin >> department;
            cout << endl;

            // encrypt username and password
            encryptedUsername = encrypt(username);
            encryptedPassword = encrypt(password);
            message = encryptedUsername + ',' + encryptedPassword + ',' + department;

            if (send(clientSocket, message.c_str(), strlen(message.c_str()), 0) < 0){
                cerr << "Send failed\n";
                exit(1);
            }

            cout << "Client has sent message " << message << " to Main Server using TCP over port "
                << dynamicPort << '.' << endl;
            
            // clean buffer
            memset(buffer, 0, sizeof(buffer));

            // Receive the reponse from the server
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived < 0){
                cerr << "Failed to receive response from server" << endl;
                exit(1);
            } 

            string response = buffer;
            if (response == "SUCCESS"){
                if (password.size() > 0){
                    cout << "Welcome member " << username << " from " << department << "!" << endl;
                }
                else{
                    cout << "Welcome guest " << username << " from " << department << "!" << endl;
                }
                cout << "Please enter the dormitory type: ";
                cin >> dormitoryType;
            }
            else{
                cout << "Failed login. Invalid username/password";
                continue;
            }
        

            cout << "-----Start a new query-----" << endl;
            memset(buffer, 0, sizeof(buffer));
        }

        close(clientSocket);
    }
};

int main(){
    Client client;

    // booting up
    client.start();
    
    return 0;
}