#include "servermain.h"

using namespace std;

const int PORT = 24778;
const int MAX_CLIENTS = 3;

int activeClients = 0;
int nextCliendID = 1;
int serverSocket;
string serverIDs;

set<int> availableIDs = {1, 2, 3}; // set to keep track of available IDs
map<int, unordered_set<string>> campusServerID;

mutex mtx; // Mutex for thread safety

// Function to split a string by a delimiter
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);

    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to load department data from a file
map<int, unordered_set<string>> loadDepartmentData(const string& filename) {
    ifstream myfile(filename);
    map<int, unordered_set<string>> map_;

    if (!myfile.is_open()) {
        cout << "File does not exist." << endl;
        return map_;  // Return empty map
    }

    int key = -1;
    string line;
    
    while (getline(myfile, line)) {
        // Trim leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;  // Skip empty lines

        // Check if the line is a key (contains only digits)
        if (line.find_first_not_of("0123456789") == string::npos) {
            key = stoi(line);
        } else {
            // This line contains values
            if (key != -1) {
                vector<string> values = split(line, ';');
                for (const string& v : values) {
                    map_[key].insert(v);
                }
            } else {
                cout << "Error: Found values before a valid key" << endl;
            }
        }
    }

    myfile.close();
    return map_;
}

void collectSeverID(){
    for (const auto& pair : campusServerID)
        serverIDs += to_string(pair.first) + ',';
    serverIDs.pop_back();
}

// Function to create and bind the socket
int createSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Bind failed\n";
        return -1;
    }

    return serverSocket;
}

// Function to handle client communication
void handleClient(int clientSocket, int ClientID) {
    char buffer[1024] = { 0 };
    
    while (true) {
        // receive a message from the client
        int byteReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (byteReceived < 0) {
            cerr << "Receive failed\n";
            break;
        } else if (byteReceived == 0) {
            cout << "Client disconnected\n";
            break;
        }

        string receivedMsg(buffer);
        int serverID = -1;
        // Search for the department in the map
        for (const auto& pair : campusServerID) {
            const auto& innerSet = pair.second;
            if (innerSet.find(receivedMsg) != innerSet.end()) {
                serverID = pair.first;
                break;  // Exit loop once found
            }
        }

        string response;
        // Confirm receiving the input from the client
        cout << "Main server has received the request on Department " 
            << receivedMsg << " from client " << ClientID << " using TCP over port " << PORT << endl;

        if (serverID == -1) {
            response = receivedMsg + " not found.";
            // server on screen msg
            cout << receivedMsg << " does not show up in Campus server " << serverIDs << endl;
            cout << "The Main server has sent 'Department Name: Not found'" 
                << " to client " << ClientID << " using TCP over port " << PORT << endl;
            cout << endl;
        } else {
            response = "Client has received results from Main Server: \n" 
                    + receivedMsg + " is associated with Campus server " + to_string(serverID) + '.';
            // server on screen msg
            cout << receivedMsg << " shows up in Campus server " << serverID << endl;
            cout << "Main Server has sent searching result to client " << ClientID 
                << " using TCP over port " << PORT << endl;
            cout << endl;
        }

        // clear the buffer and send a reponse to the client
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, response.c_str());

        if (send(clientSocket, buffer, response.size(), 0) < 0){
            cerr << "Failed to send reponse to client\n";
            break;
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

int main() {
    // Load department data from file
    campusServerID = loadDepartmentData("list.txt");
    collectSeverID();

    // Create and bind the socket
    serverSocket = createSocket(PORT);
    if (serverSocket == -1) return 1;

    // Listening for connections
    if (listen(serverSocket, 3) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Main server is up and running." << endl;
    cout << "Main server has read the department list from list.txt." << endl;
    cout << endl;

    // print the counting results of campusServerID
    cout << "Total num of Campus Servers: " << campusServerID.size() << endl;
    for (const auto& pair : campusServerID){
        cout << "Campus Server " << pair.first << " contains " << pair.second.size() << " distinct departments" << endl;
    }
    cout << endl;
    // keep accepting and handling client connections in a loop
    while (true) {
        // Accepting connection
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "Accept failed\n";
            continue;
        }

        // Lock the mutex to modify the active client count
        mtx.lock();
        
        if (activeClients < MAX_CLIENTS) {
            int clientID = *availableIDs.begin();
            availableIDs.erase(availableIDs.begin());
            ++activeClients;
            cout << "Client connected. Active clients: " << clientID << endl;

            pid_t pid = fork(); // create a new process
            if (pid < 0){
                cerr << "Fork failed\n";
                close(clientSocket);
                mtx.unlock();
                continue;
            } else if (pid == 0){
                // cliend process
                /* the child process inherits copies of all file descriptors from the parent, 
                including the serverSocket. If the child process continues to keep the serverSocket open, 
                it will hold a reference to it, which may lead to unnecessary resource consumption.
                */
                close(serverSocket);
                handleClient(clientSocket, clientID);
                exit(0);
            } else{
                close(clientSocket);
            }

        } else{
            cerr << "Maximum clients reached. Rejecting connection\n";
            close(clientSocket);
        }
        mtx.unlock();
        
        
    }
    // close server
    close(serverSocket);

    return 0;
}