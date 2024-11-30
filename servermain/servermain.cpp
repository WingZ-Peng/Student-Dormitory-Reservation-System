#include "servermain.h"
using namespace std;

#define CAMPUS_SERVER_PORT_A 31778
#define CAMPUS_SERVER_PORT_B 32778
#define CAMPUS_SERVER_PORT_C 33778
#define MAIN_SERVER_UDP_PORT 34778
#define MAIN_SERVER_TCP_PORT 35778
#define BUFFER_SIZE 1024

class MainServer {
private:
    unordered_map<string, string> member_encrypted_info_;
    unordered_map<string, int> department_campus_mapping_;
    unordered_map<int, struct sockaddr_in> campus_server_address_mapping_;

    void readLoginInfo(const string& path) {
        ifstream file(path);
        if (!file.is_open()) {
            cerr << "Open file failed: " << path << endl;
            exit(1); 
        }

        string line;
        
        while (getline(file, line)) {
            stringstream ss(line);
            string username, password;
            getline(ss, username, ',');
            getline(ss, password, ',');
            member_encrypted_info_[username] = password;
        }
        file.close();
    }

    bool validateLoginInfo(const string& username, const string& password) {
        cout << "The main server received the authentication for "
            << username << " using TCP over port " << MAIN_SERVER_TCP_PORT << '.' << endl;
        
        if (member_encrypted_info_.find(username) != member_encrypted_info_.end()) {
            if (member_encrypted_info_[username] == password) {
                return true;
            }
        }

        return false;
    }

    void print() const {
        cout << endl;
        cout << "Member encrypted information" << endl;
        
        for (const auto& pair : member_encrypted_info_) {
            cout << pair.first << ',' << pair.second << endl; 
        }
        cout << endl;
    }

    void persistDepartmentList(char buffer[], string& department_list) {
        int campus_id = buffer[0] - 'A';
        const char* ptr = buffer + 2;
        const char* end = buffer + strlen(buffer);

        while (ptr < end) {
            const char* comma = strchr(ptr, ',');

            if (comma) {
                string department_name(ptr, comma - ptr);
                department_campus_mapping_.emplace(department_name, campus_id);
                department_list += department_name + ", ";
                ptr = comma + 1;
            } 
            else {
                department_campus_mapping_.emplace(string(ptr), campus_id);
                department_list += string(ptr);
                break;
            }
        }
    }

    void processDepartmentList(int sockfd, int campus_port) {
        sockaddr_in campus_address;

        // Configure server address
        memset(&campus_address, 0, sizeof(campus_address));
        campus_address.sin_family = AF_INET;
        campus_address.sin_port = htons(campus_port);
        inet_pton(AF_INET, "127.0.0.1", &campus_address.sin_addr);
        campus_server_address_mapping_[campus_port] = campus_address;

        string message = "DEPARTMENT_LIST";
        sendto(sockfd, message.c_str(), message.size(), 0, (const sockaddr*)&campus_address, sizeof(campus_address));

        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(campus_address);
        int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&campus_address, &len);
        buffer[bytes_received] = '\0';

        // Print on-screen message receiving the department lists from Campus server
        string department_list = "Server " + string(1, buffer[0]) + ": ";
        cout << "Main server has received the department list from server " << string(1, buffer[0])
             << " using UDP over port " << MAIN_SERVER_UDP_PORT << endl;

        persistDepartmentList(buffer, department_list);
        cout << department_list << endl;
        cout << endl;
    }

    int createUdpSocket() {
        int udp_socket;
        struct sockaddr_in udp_address;

        // Create socket by using UDP
        udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket == -1) {
            cerr << "Socket creation failed" << endl;
            exit(1);
        }

        // Configure server address
        memset(&udp_address, 0, sizeof(udp_address));
        udp_address.sin_family = AF_INET;
        udp_address.sin_port = htons(MAIN_SERVER_UDP_PORT);
        inet_pton(AF_INET, "127.0.0.1", &udp_address.sin_addr);

        // Bind the socket
        if (::bind(udp_socket, (const struct sockaddr*)&udp_address, sizeof(udp_address)) < 0) {
            cerr << "Bind failed" << endl;
            close(udp_socket);
            exit(1);
        }

        return udp_socket;
    }

    int createTcpSocket() {
        int tcp_socket;
        struct sockaddr_in tcp_address;

        // Create socket by using TCP
        tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_socket == -1) {
            cerr << "Socket creation failed" << endl;
            exit(1);
        }

        // Configure server address
        memset(&tcp_address, 0, sizeof(tcp_address));
        tcp_address.sin_family = AF_INET;
        tcp_address.sin_port = htons(MAIN_SERVER_TCP_PORT);
        inet_pton(AF_INET, "127.0.0.1", &tcp_address.sin_addr);

        // Bind the socket
        if (::bind(tcp_socket, (const struct sockaddr*)&tcp_address, sizeof(tcp_address)) < 0) {
            cerr << "Bind failed" << endl;
            close(tcp_socket);
            exit(1);
        }

        // Listen for incoming connections
        if (listen(tcp_socket, 5) < 0) {
            cerr << "Listen failed" << endl;
            exit(1);
        }

        return tcp_socket;
    }

    void handleClientConnection(int client_socket, int udp_socket) {
        char buffer[BUFFER_SIZE];

        while (true) {
            // Read data from client
            memset(buffer, 0, sizeof(buffer));
            int bytes_received_from_client = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received_from_client < 0) {
                cerr << "Read failed from client" << endl;
                close(client_socket);
                continue;
            }

            string authentication_result;
            string authentication_info(buffer);
            size_t comma_pos = authentication_info.find(',');
            string client_type = authentication_info.substr(0, comma_pos);
            authentication_info = authentication_info.substr(comma_pos + 1);
            comma_pos = authentication_info.find(',');
            string username = authentication_info.substr(0, comma_pos);
            string password = authentication_info.substr(comma_pos + 1);

            if (validateLoginInfo(username, password)) {
                cout << "The authentication passed.";
                authentication_result = "PASSED";
            }
            else {
                cout << "The authentication failed";
                authentication_result = "FAILED";
            }
            cout << endl;

            // send validation result back to client
            if (send(client_socket, authentication_result.c_str(), authentication_result.size(), 0) < 0) {
                cerr << "Send authtication result to client failed" << endl;
                close(client_socket);
                continue;
            }
            cout << "The main server sent the authentication result to the client." << endl;

            // prepare for next query
            memset(buffer, 0, sizeof(buffer));
            bytes_received_from_client = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received_from_client < 0) {
                cerr << "Read failed from client (second)" << endl;
                close(client_socket);
                continue;
            }

            // Disassemble query 
            string query(buffer);
            comma_pos = query.find(',');
            string department = query.substr(0, comma_pos);
            query = query.substr(comma_pos + 1);
            comma_pos = query.find(',');
            string action = query.substr(0, comma_pos);
            cout << "Main server has received the query from " << client_type << " " << username
                << " in " << department << " for the request of " << action << endl;
            
            // verify department
            if (department_campus_mapping_.find(department) == department_campus_mapping_.end()) {
                cout << department << " does not show up in Campus servers." << endl;
                continue;
            }

            // Search campus server address
            string server_name;
            int campus_port = department_campus_mapping_[department];

            if (campus_port == 0) {
                campus_port = CAMPUS_SERVER_PORT_A;
                server_name = "A";
            } else if (campus_port == 1) { 
                campus_port = CAMPUS_SERVER_PORT_B;
                server_name = "B";
            } else { 
                campus_port = CAMPUS_SERVER_PORT_C;
                server_name = "C";
            }
            sockaddr_in campus_address = campus_server_address_mapping_[campus_port];

            // Send query to campus server
            sendto(udp_socket, query.c_str(), query.size(), 0, (const struct sockaddr*)&campus_address, sizeof(campus_address));
            cout << "The main server forwarded a request of " << action << " to Server " << server_name << " using UDP over port "
                << MAIN_SERVER_UDP_PORT << endl;
            cout << endl;
            
            // Clean buffer
            memset(buffer, 0, sizeof(buffer));

            // Receive response from the campus server
            socklen_t len = sizeof(campus_address);
            int bytes_received_from_campus_server = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&campus_address, &len);
            buffer[bytes_received_from_campus_server] = '\0';
            string response(buffer);

            cout << "The Main server has received result for the request of " << action << " from Campus server "
                << server_name << " using UDP over port " << MAIN_SERVER_UDP_PORT << endl;

            // send back to client
            if (send(client_socket, response.c_str(), response.size(), 0) < 0) {
                cerr << "Response to client failed" << endl;
                close(client_socket);
                continue;
            }
            cout << "The Main server has sent back the result for the request of " << action << " to the client "
                << client_type << " " << username << " using TCP over port " << MAIN_SERVER_TCP_PORT << endl;

            memset(buffer, 0, sizeof(buffer));
        }
    }

public:
    MainServer(const string& file_path) {
        readLoginInfo(file_path);
        // print();
    }

    void start() {
        int udp_socket = createUdpSocket();
        int tcp_socket = createTcpSocket();

        // Get department list info at first
        processDepartmentList(udp_socket, CAMPUS_SERVER_PORT_A);
        // processDepartmentList(udp_socket, CAMPUS_SERVER_PORT_B);
        // processDepartmentList(udp_socket, CAMPUS_SERVER_PORT_C);

        // Handle tcp connection
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        while (true) {
            int client_socket = accept(tcp_socket, (sockaddr*)&client_address, &client_len);
            if (client_socket < 0) {
                cerr << "Listen to client failed" << endl;
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                cerr << "Fork TCP process failed" << endl;
                exit(1);
            }
            else if (pid == 0) {
                handleClientConnection(client_socket, udp_socket);
                close(client_socket);
                close(udp_socket);
                exit(0);
            }
        }

        close(udp_socket);
        close(tcp_socket);
    }
};

int main() {
    string file_path = "../loginInfo/member.txt";
    MainServer main_server(file_path);
    
    cout << "Main server is up and running." << endl << endl;
    main_server.start();

    return 0;
}
