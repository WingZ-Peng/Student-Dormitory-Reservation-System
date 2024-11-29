#include "client.h"
using namespace std;

#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class Client {
private:
    int dynamic_port_;

    // Encryption function
    string encrypt(const string& input) {
        if (input.empty()) return input;
        string encrypted = input;

        for (char& c : encrypted) {
            if (isupper(c)) {
                c = 'A' + (c - 'A' + 3) % 26;
            } 
            else if (islower(c)) {
                c = 'a' + (c - 'a' + 3) % 26;
            } 
            else if (isdigit(c)) {
                c = '0' + (c - '0' + 3) % 10;
            }
        }

        return encrypted;
    }

    int createClientSocket() {
        int client_socket;
        struct sockaddr_in server_address;

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

        // Send connection request
        if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
            cerr << "Connection failed" << endl;
            exit(1);
        }

        socklen_t address_length = sizeof(server_address);
        if (getsockname(client_socket, (sockaddr*)&server_address, &address_length) == -1) {
            cerr << "Failed to get socket name" << endl;
            exit(EXIT_FAILURE);
        }
        dynamic_port_ = ntohs(server_address.sin_port);

        return client_socket;
    }

    bool isValidUsername(const string& username) {
        if (username.size() < 5 || username.size() > 50) {
            cerr << "Error: Username must be between 5 and 50 lowercase characters." << endl;
            return false;
        }
        for (const char& c : username) {
            if (!islower(c)) {
                cerr << "Error: Username must contain only lowercase characters." << endl;
                return false;
            }
        }

        return true;
    }

    bool isValidPassword(const string& password) {
        if (password.size() < 5 || password.size() > 50) {
            cerr << "Error: Password must be between 5 and 50 lowercase characters." << endl;
            return false;
        }
    
        return true;
    }

public:
    void start() {
        int client_socket = createClientSocket();
        char buffer[BUFFER_SIZE];

        cout << "Client is up and running" << endl;
        while (true) {
            string username, password, department;
            string encrypted_username, encrypted_password;
            string query, dormitory_type, action, building_id;

            // Input username
            cout << "Enter user name: ";
            cin.ignore();
            getline(cin, username);
            if (!isValidUsername(username)) continue;

            // Input password
            cout << "Enter password: ";
            cin.ignore();
            getline(cin, password);
            if (!isValidPassword(password)) continue;

            // Input department name
            cout << "Enter department name: ";
            cin.ignore();
            getline(cin, department);
            cout << endl;

            // Encrypt username and password
            encrypted_username = encrypt(username);
            encrypted_password = encrypt(password);
            query = "Member" + encrypted_username + ',' + encrypted_password;

            // Send query for validation
            if (send(client_socket, query.c_str(), query.size(), 0) < 0) {
                cerr << "Send username and password failed" << endl;
                exit(1);
            }

            cout << "Member has sent message " << query << " to Main Server using TCP over port "
                 << dynamic_port_ << '.' << endl;

            // Receive the response from the server
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                cerr << "Receive response from server failed" << endl;
                exit(1);
            }

            string response(buffer);
            if (response == "SUCCESS") {
                cout << "Welcome member " << username << " from " << department << "!" << endl;
            } else {
                cout << "Failed login. Invalid username/password" << endl;
                continue;
            }

            // Input dormitory type
            cout << "Please enter the dormitory type: ";
            cin.ignore();
            getline(cin, dormitory_type);

            // Client can choose one of three different types of query actions
            cout << "Please enter request action (availability, price, reserve): ";
            cin.ignore();
            getline(cin, action);

            // Validate the permission
            if (action != "availability" && action != "price" && action != "reserve") {
                cout << "Action not found" << endl;
                continue;
            }

            // Prepare query
            query = department + ',' + dormitory_type + ',' + action;

            if (action == "reserve") {
                cout << "Please enter Building ID for reservation: ";
                cin.ignore();
                getline(cin, building_id);
                query += ',' + building_id;
            }

            // Send query
            if (send(client_socket, query.c_str(), query.size(), 0) < 0) {
                cerr << "Send department and dormitory type failed" << endl;
                exit(1);
            }

            // Receive response from the server
            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                cerr << "Receive response from server failed" << endl;
                exit(1);
            }

            response = buffer;
            cout << response << endl;

            cout << "-----Start a new query-----" << endl;
        }

        close(client_socket);
    }
};

int main() {
    Client client;
    client.start();

    return 0;
}
