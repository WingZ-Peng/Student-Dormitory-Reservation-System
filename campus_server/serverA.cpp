#include "server.h"
using namespace std;

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
    const int PORT = 24778;
    const int BUFFER_SIZE = 1024;

    string filePath = "../data/dataA.txt";
    vector<string> departments;
    unordered_map<string, int> availabilityCount;
    unordered_map<string, unordered_set<string> > buildingIds;
    // read data
    readData(filePath, departments, availabilityCount, buildingIds);
    // print out info
    // print(filePath, departments, availabilityCount, buildingIds);

    int len;
    int serverSocket;
    char buffer[BUFFER_SIZE];
    sockaddr_in serverAddress;
    socklen_t length;

    // create socket by using UDP
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1){
        cerr << "Error creating socket\n";
        return -1;
    }

    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_port        = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    string message = "Hello from client";
    sendto(serverSocket, message.c_str(), message.size(), 0, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
    cout << "Message sent to server." << endl;

    len = recvfrom(serverSocket, (char*)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&serverAddress, &length);
    buffer[len] = '\0';
    cout << "Server: " << buffer << endl;
    
    close(serverSocket);
    return 0;
}

