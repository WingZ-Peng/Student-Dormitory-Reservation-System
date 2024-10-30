#include "servermain.h"
using namespace std;

#define MAIN_SERVER_PORT 33778
#define CAMPUS_SERVER_PORT_A 30778
#define CAMPUS_SERVER_PORT_B 31778
#define CAMPUS_SERVER_PORT_C 32778
#define BUFFER_SIZE 1024

class MainServer{
private:
    unordered_map<string,int> departmentCampusMapping;

    void getDepartmentList(int sockfd, int campusPort){
        struct sockaddr_in campusAddress;

        // configure server address
        memset(&campusAddress, 0, sizeof(campusAddress));
        campusAddress.sin_family = AF_INET;
        campusAddress.sin_port = htons(campusPort);
        inet_pton(AF_INET, "127.0.0.1", &campusAddress.sin_addr);

        string MSG = "DEPARTMENT_LIST";
        sendto(sockfd, MSG.c_str(), MSG.size(), 0, (const struct sockaddr *)&campusAddress, sizeof(campusAddress));

        char buffer[BUFFER_SIZE];
        socklen_t len = sizeof(campusAddress);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&campusAddress, &len);
        buffer[n] = '\0';
        cout << "Received: " << buffer << " on port " << campusPort << endl;

        // persist the department list
        int val = buffer[0] - 'A';
        const char* ptr = buffer + 2;
        const char* end = buffer + strlen(buffer);

        while (ptr < end) {
            const char* comma = strchr(ptr, ',');

            if (comma){
                departmentCampusMapping.emplace(string(ptr, comma - ptr), val);
                ptr = comma + 1;
            }
            else{
                departmentCampusMapping.emplace(string(ptr), val);
                break;
            }
        }

        for (const auto& pair : departmentCampusMapping) {
            std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
        }
    }

public:
    void start(){
        int sockfd;
        struct sockaddr_in serverAddress;
        char buffer[BUFFER_SIZE];

        // create socket by using UDP
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1){
            cerr << "socket creation failed" << endl;
            exit(1);
        }

        // configure server address
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(MAIN_SERVER_PORT);
        inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

        // build the socket
        if (::bind(sockfd, (const struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
            cerr << "bind failed" << endl;
            close(sockfd);
            exit(1);
        }

        // get department list info at first
        getDepartmentList(sockfd, CAMPUS_SERVER_PORT_A);
        getDepartmentList(sockfd, CAMPUS_SERVER_PORT_B);
        getDepartmentList(sockfd, CAMPUS_SERVER_PORT_C);

        // stand by for further queries
        while (true){
            string department;
            cout << "Enter department name: ";
            cin >> department;
        }

        close(sockfd);
    }
};

int main(){
    MainServer mainServer;
    mainServer.start();
    
    return 0;
}
