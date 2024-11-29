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
    unordered_map<string, int> department_campus_mapping_;
    unordered_map<int, struct sockaddr_in> campus_server_address_mapping_;

    void ProcessDepartmentList(int sockfd, int campus_port) {
        struct sockaddr_in campus_address;

        // Configure server address
        memset(&campus_address, 0, sizeof(campus_address));
        campus_address.sin_family = AF_INET;
        campus_address.sin_port = htons(campus_port);
        inet_pton(AF_INET, "127.0.0.1", &campus_address.sin_addr);
        campus_server_address_mapping_[campus_port] = campus_address;

        string message = "DEPARTMENT_LIST";
        sendto(sockfd, message.c_str(), message.size(), 0, (const struct sockaddr *)&campus_address, sizeof(campus_address));

        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(campus_address);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&campus_address, &len);
        buffer[n] = '\0';

        // Print on-screen message receiving the department lists from Campus server
        string department_list = "Server " + string(1, buffer[0]) + ": ";
        cout << "Main server has received the department list from Campus server " << string(1, buffer[0])
             << " using UDP over port " << MAIN_SERVER_UDP_PORT << endl;

        // Persist the department list
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
            } else {
                department_campus_mapping_.emplace(string(ptr), campus_id);
                department_list += string(ptr);
                break;
            }
        }
        cout << department_list << endl;
        cout << endl;
    }

    int CreateUdpSocket() {
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

    int CreateTcpSocket() {
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

    
public:
    void Start() {
        // UDP & TCP setup
        int udp_socket = CreateUdpSocket();
        int tcp_socket = CreateTcpSocket();
        
        // Get department list info at first
        ProcessDepartmentList(udp_socket, CAMPUS_SERVER_PORT_A);
        ProcessDepartmentList(udp_socket, CAMPUS_SERVER_PORT_B);
        ProcessDepartmentList(udp_socket, CAMPUS_SERVER_PORT_C);

        // Handle tcp & udp connection
        int client_socket;
        struct sockaddr_in client_address, campus_address;
        socklen_t client_len = sizeof(client_address);
        char buffer[BUFFER_SIZE];
        
        while (true) {
            client_socket = accept(tcp_socket, (struct sockaddr*)&client_address, &client_len);
            if (client_socket < 0) {
                cerr << "Accept failed" << endl;
                continue;
            }

            cout << "Accepted new client connection." << endl;

            // Read data from client
            memset(buffer, 0, sizeof(buffer));
            int bytes_received_from_client = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received_from_client < 0) {
                cerr << "Read failed from client" << endl;
                close(client_socket);
                continue;
            }

            cout << "Received from client: " << buffer << endl;

            // Send query to campus server
            string query(buffer);
            campus_address = campus_server_address_mapping_[CAMPUS_SERVER_PORT_A];

            // Send query
            sendto(udp_socket, query.c_str(), query.size(), 0, (const struct sockaddr*)&campus_address, sizeof(campus_address));

            // Clean buffer
            memset(buffer, 0, sizeof(buffer));

            // Receive response from the campus server
            socklen_t len = sizeof(campus_address);
            int bytes_received_from_campus_server = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&campus_address, &len);
            buffer[bytes_received_from_campus_server] = '\0';
            string response(buffer);

            cout << response << endl;

            // Send result to client
            if (send(client_socket, response.c_str(), strlen(response.c_str()), 0) < 0) {
                cerr << "Response failed" << endl;
                exit(1);
            }

            memset(buffer, 0, sizeof(buffer));
        }

        close(udp_socket);
        close(tcp_socket);
    }
};

int main() {
    MainServer main_server;

    // Booting up
    cout << "Main server is up and running." << endl;
    cout << endl;
    main_server.Start();
    
    return 0;
}