#include "server.h"
using namespace std;

#define CAMPUS_SERVER_PORT_C 32778
#define MAIN_SERVER_PORT 33778
#define BUFFER_SIZE 1024

class CampusServerC{
private:
    vector<string> departments;
    unordered_map<string, int> availabilityCount;
    unordered_map<string, unordered_set<string> > buildingIds;

    void readData(const string& filePath){
        ifstream file(filePath);
        if (!file.is_open()){
            cerr << "Failed to open file: " << filePath << endl;
            return;
        }

        string line;
        // read the first line
        if (getline(file, line)){
            stringstream ss(line);
            string department;
            while (getline(ss, department, ',')){
                department = Utility::trim(department);
                departments.push_back(department);
            }
        }

        while (getline(file, line)){
            stringstream ss(line);
            string type, buildingId, availabilityStr, price;
            getline(ss, type, ',');
            getline(ss, buildingId, ',');
            getline(ss, availabilityStr, ',');
            getline(ss, price, ',');
            // update availability count
            int availailty = stoi(availabilityStr);
            availabilityCount[type] += availailty;
            // update building ids
            buildingIds[type].insert(buildingId);
        }
        file.close();
    }
public:
    CampusServerC(const string& filePath){
        readData(filePath);
    }

    void start(){
        int sockfd;
        char buffer[BUFFER_SIZE];
        struct sockaddr_in serverAddress, mainServerAddress;

        // create socket by using UDP
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1){
            cerr << "socket creation failed" << endl;
            exit(1);
        }

        // configure server address
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(CAMPUS_SERVER_PORT_C);
        inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

        // bind
        if (::bind(sockfd, (const struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
            cerr << "bind failed" << endl;
            close(sockfd);
            exit(1);
        }

        // Wait for wake-up message from Main Server
        socklen_t len = sizeof(mainServerAddress);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&mainServerAddress, &len);

        // send department list to main server
        string response = "C";

        for (const auto& department :departments){
            response += ',' + department;
        }
        sendto(sockfd, response.c_str(), response.length(), 0, (const struct sockaddr *)&mainServerAddress, len);

        // Stand by for further queries
        while (true)
        {
            n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&mainServerAddress, &len);
            buffer[n] = '\0';
            std::string query(buffer);
            // Handle query and send response
        }

        close(sockfd);
    }
};

int main(){
    string filePath = "../data/dataC.txt";
    CampusServerC server(filePath);
    server.start();
    
    return 0;
}

