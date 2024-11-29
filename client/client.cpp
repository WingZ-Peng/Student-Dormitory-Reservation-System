#include "client.h"
using namespace std;

#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class Client {
private:
    // Encryption function
    string Encrypt(const string& input) {
        if (input.empty()) {return input; }
        string encrypted = input;

        for (char& c : encrypted) {
            if (c >= 'A' && c <= 'Z') {
                c = 'A' + (c - 'A' + 3) % 26;
            } 
            else if (c >= 'a' && c <= 'z') {
                c = 'a' + (c - 'a' + 3) % 26;
            } 
            else if (c >= '0' && c <= '9') {
                c = '0' + (c - '0' + 3) % 26;
            }
        }
        
        return encrypted;
    }

public:
    void Start() {
        int client_socket;
        struct sockaddr_in server_address;
        socklen_t address_length = sizeof(server_address);
        char buffer[BUFFER_SIZE];

        // Create socket by using TCP
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            cerr << "Socket creation failed" << endl;
            exit(1);
        }

        // Configure server address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(MAIN_SERVER_TCP_PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

        // Send connect request
        if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            cerr << "Connection failed" << endl;
            exit(1);
        }

        // Get the port number assigned by the OS
        if (getsockname(client_socket, (struct sockaddr *)&server_address, &address_length) == -1) {
            cerr << "failed to get socket name" << endl;
            exit(EXIT_FAILURE);
        }
        int dynamic_port = ntohs(server_address.sin_port);

        cout << "Client is up and running" << endl;
        while (true) {
            string username, password, department;
            string encrypted_username, encrypted_password;
            string message, dormitory_type;
            
            // Input username
            cout << "Enter user name: ";
            cin >> username;
            // Letter number constraint
            if (username.size() < 5 || username.size() > 50) {
                cerr << "Error: Username must be between 5 and 50 lowercase characters." << endl;
                continue;
            }
            // Lower letter constraint
            bool valid_username = true;
            for (const char& c : username) {
                if (!islower(c)) {
                    cerr << "Error: Username must contain only lowercase characters." << endl;
                    valid_username = false;
                    break;
                }
            }
            if (!valid_username) continue;

            // Input password
            cout << "Enter password: ";
            cin >> password;
            // Letter number constraint
            if (password.size() < 5 || password.size() > 50) {
                cerr << "Error: Password must be between 5 and 50 lowercase characters." << endl;
                continue;
            }

            // Input department name
            cout << "Enter department name: ";
            cin >> department;
            cout << endl;

            // Encrypt username and password
            encrypted_username = Encrypt(username);
            encrypted_password = Encrypt(password);
            message = encrypted_username + ',' + encrypted_password + ',' + department;

            if (send(client_socket, message.c_str(), strlen(message.c_str()), 0) < 0) {
                cerr << "Send failed" << endl;
                exit(1);
            }

            cout << "Client has sent message " << message << " to Main Server using TCP over port "
                 << dynamic_port << '.' << endl;
            
            // Clean buffer
            memset(buffer, 0, sizeof(buffer));

            // Receive the response from the server
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                cerr << "Failed to receive response from server" << endl;
                exit(1);
            } 

            string response(buffer);
            if (response == "SUCCESS") {
                if (password.size() > 0) {
                    cout << "Welcome member " << username << " from " << department << "!" << endl;
                } else {
                    cout << "Welcome guest " << username << " from " << department << "!" << endl;
                }
                cout << "Please enter the dormitory type: ";
                cin >> dormitory_type;
            } else {
                cout << "Failed login. Invalid username/password";
                continue;
            }
        
            cout << "-----Start a new query-----" << endl;
            memset(buffer, 0, sizeof(buffer));
        }

        close(client_socket);
    }
};

int main() {
    Client client;

    // Booting up
    client.Start();
    
    return 0;
}
