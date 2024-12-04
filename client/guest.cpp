#include "client.h"
using namespace std;

#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class Guest {
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

public:
    void start() {
        int client_socket = createClientSocket();
        char buffer[BUFFER_SIZE];

        while (true) {
            string username, password, department;
            string encrypted_username, encrypted_password;
            string query, dormitory_type, action, building_id;

            // Input username
            cout << "Please enter username: ";
            getline(cin, username);

            // Input password
            cout << "Please enter password: ";
            getline(cin, password);

            if (password.empty()) {
            cout << "Password skipped." << endl;
            }

            // Input department name
            cout << "Please enter department name: ";
            getline(cin, department);
            cout << endl;

            // Encrypt username and password
            encrypted_username = encrypt(username);
            query = "Guest," + encrypted_username;

            // Send query for validation
            if (send(client_socket, query.c_str(), query.size(), 0) < 0) {
                cerr << "Send username and password failed" << endl;
                exit(1);
            }

            cout << username << " sent an authentication request to the main server." << endl;

            // Receive the response from the server
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                cerr << "Receive response from server failed" << endl;
                exit(1);
            }

            string response(buffer);
            if (response == "PASSED") {
                cout << "Welcome guest " << username << " from " << department << "!" << endl;
            } 

            // Input dormitory type
            cout << "Please enter the room type S/D/T: ";
            getline(cin, dormitory_type);

            // Guest only can choose availability 
            while (action != "availability") {
                cout << "Please enter request action (availability, price, reserve): ";
                getline(cin, action);
                // Validate the permission
                if (action != "availability") {
                    cout << "Permission denied, " + action + " is member client only." << endl;
                }
            }

            // Prepare query
            query = department + ',' + action + ',' + dormitory_type;

            if (action == "availability") {
                cout << username << " sent a request of Availability for type "
                    << dormitory_type << " to the main server." << endl;
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
            cout << "The client received the response from the main server using TCP over port "
                    << dynamic_port_ << '.' << endl;
            cout << endl;
            // print out the on screen message
            response = buffer;
            cout << response << endl;
            // start new query
            cout << endl;
            cout << "-----Start a new query-----" << endl;
        }

        close(client_socket);
    }
};

int main() {
    Guest guest;

    cout << "Client is up and running." << endl;
    guest.start();

    return 0;
}
