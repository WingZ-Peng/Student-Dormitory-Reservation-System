#include "client.h"
using namespace std;

#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class Guest {
private:
    int dynamic_port_;

    int createGuestSocket() {
        int guest_socket;
        struct sockaddr_in server_address;

        // Create socket by using TCP
        guest_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (guest_socket == -1) {
            cerr << "Guest socket creation failed" << endl;
            exit(1);
        }

        // Configure server address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(MAIN_SERVER_TCP_PORT);
        inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

        // Send connection request
        if (connect(guest_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
            cerr << "Connection failed" << endl;
            exit(1);
        }

        socklen_t address_length = sizeof(server_address);
        if (getsockname(guest_socket, (sockaddr*)&server_address, &address_length) == -1) {
            cerr << "Failed to get socket name" << endl;
            exit(EXIT_FAILURE);
        }
        dynamic_port_ = ntohs(server_address.sin_port);

        return guest_socket;
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

public:
    void start() {
        int guest_socket = createGuestSocket();
        char buffer[BUFFER_SIZE];

        cout << "Guest is up and running" << endl;
        while (true) {
            string username, department;
            string query, dormitory_type;

            // Input username
            cout << "Enter user name: ";
            cin.ignore();
            getline(cin, username);
            if (!isValidUsername(username)) continue;

            // Input department name
            cout << "Enter department name: ";
            cin.ignore();
            getline(cin, department);
            cout << endl;
            cout << "Welcome guest " << username << " from " << department << "!" << endl;

            // Input dormitory type
            cout << "Please enter the dormitory type: ";
            cin.ignore();
            getline(cin, dormitory_type);

            // Prepare query
            query = department + ',' + dormitory_type + ',' + "availability";

            // Send query
            if (send(guest_socket, query.c_str(), query.size(), 0) < 0) {
                cerr << "Send department and dormitory type failed" << endl;
                exit(1);
            }

            // Receive response from the server
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(guest_socket, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                cerr << "Receive response from server failed" << endl;
                exit(1);
            }

            string response(buffer);
            cout << response << endl;

            cout << "-----Start a new query-----" << endl;
        }

        close(guest_socket);
    }
};

int main() {
    Guest guest;
    guest.start();

    return 0;
}
