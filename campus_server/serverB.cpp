#include "server.h"
using namespace std;

#define serverName "B"
#define PORT 31778
#define BUFFER_SIZE 1024

// Helper function to trim whitespace from both ends of a string
string trim(const string& s) {
    auto start = s.begin();
    while (start != s.end() && isspace(*start)) {
        start++;
    }

    auto end = s.end();
    do {
        end--;
    } while (distance(start, end) > 0 && isspace(*end));

    return string(start, end + 1);
}

void readData(const string& filePath, vector<string>& departments, unordered_map<string, int>& availabilityCount, unordered_map<string, unordered_set<string> >& buildingIds){
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
            department = trim(department);
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

void print(const string& filePath, const vector<string>& departments, const unordered_map<string, int>& availabilityCount, unordered_map<string, unordered_set<string> >& buildingIds){
    // Print departments
    cout << "Departments:" << endl;
    for (const auto& department : departments) {
        cout << department << " ";
    }
    cout << endl;

    // Print availability count per type
    cout << "\nAvailability Count:" << endl;
    for (const auto& pair : availabilityCount) {
        cout << pair.first << ": " << pair.second << endl;
    }

    // Print building IDs per type
    cout << "\nBuilding IDs:" << endl;
    for (const auto& pair : buildingIds) {
        cout << pair.first << ": ";
        for (const auto& id : pair.second) {
            cout << id << " ";
        }
        cout << endl;
    }
}

int main(){
    string filePath = "../data/dataB.txt";
    vector<string> departments;
    unordered_map<string, int> availabilityCount;
    unordered_map<string, unordered_set<string> > buildingIds;
    // read data
    readData(filePath, departments, availabilityCount, buildingIds);
    // print out info
    // print(filePath, departments, availabilityCount, buildingIds);

    int serverSocket;
    char buffer[BUFFER_SIZE];
    sockaddr_in serverAddress, clientAddress;

    // create socket by using UDP
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1){
        cerr << "Failed to create socket for Sever " << serverName << endl;
        return -1;
    }

    // configure server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // build the socket
    if (::bind(serverSocket, (const struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        cerr << "Failed to bind socket for Sever " << serverName << endl;
        close(serverSocket);
        return -1;
    }
    
    cout << "Server " << serverName << " listening on port " << PORT << endl;

    // receive incoming datagrams from clients
    while (true){
        socklen_t length = sizeof(clientAddress);
        int bytes = recvfrom(serverSocket, (char*)buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddress, &length);
        if (bytes < 0){
        cerr << "Failed to receive response from Server " << serverName << endl;
        }
        else{
            buffer[bytes] = '\0';
            cout << "Received from Server " << serverName << ": " << buffer << endl;
        }

        // send response to the main server
        string response = "Hello from server ";
        response += serverName;
        sendto(serverSocket, response.c_str(), response.size(), 0, (struct sockaddr *)&clientAddress, length);
        cout << "Response sent to main server." << endl;
    }
    
    close(serverSocket);
    return 0;
}

