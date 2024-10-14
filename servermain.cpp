#include "servermain.h"

using namespace std;

const int PORT = 24778;

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
unordered_map<int, unordered_set<string>> loadDepartmentData(const string& filename) {
    ifstream myfile(filename);
    unordered_map<int, unordered_set<string>> campusServerID;

    if (!myfile.is_open()) {
        cout << "File does not exist." << endl;
        return campusServerID;  // Return empty map
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
                    campusServerID[key].insert(v);
                }
            } else {
                cout << "Error: Found values before a valid key" << endl;
            }
        }
    }

    myfile.close();
    return campusServerID;
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
void handleClient(int clientSocket, const unordered_map<int, unordered_set<string>>& campusServerID) {
    char buffer[1024] = { 0 };
    
    while (true) {
        // receive a message from the client
        int byteReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (byteReceived < 0) {
            cerr << "Receive failed\n";
            return;
        } else if (byteReceived == 0) {
            cout << "Client disconnected\n";
            return;
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
        if (serverID == -1) {
            response = "Department not found";
            cout << response << endl;
        } else {
            response = "Department " + string(buffer) + " is associated with Campus server " + to_string(serverID);
            cout << response << endl;
        }

        // clear the buffer and send a reponse to the client
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, response.c_str());

        if (send(clientSocket, buffer, response.size(), 0) < 0){
            cerr << "Failed to send reponse to client\n";
            return;
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

int main() {
    // Load department data from file
    unordered_map<int, unordered_set<string>> campusServerID = loadDepartmentData("list.txt");

    // Create and bind the socket
    int serverSocket = createSocket(PORT);
    if (serverSocket == -1) return 1;

    // Listening for connections
    if (listen(serverSocket, 3) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Main server is up and running" << endl;
    cout << "Main server has read the department list from list.txt." << endl;
    
    // keep accepting and handling client connections in a loop
    while (true) {
        // Accepting connection
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        
        if (clientSocket < 0) {
            cerr << "Accept failed\n";
            continue;
        }

        pid_t pid = fork(); // create a new process
        if (pid < 0){
            cerr << "Fork failed\n";
            close(clientSocket);
            continue;
        } else if (pid == 0){
            // cliend process
            /* the child process inherits copies of all file descriptors from the parent, 
               including the serverSocket. If the child process continues to keep the serverSocket open, 
               it will hold a reference to it, which may lead to unnecessary resource consumption.
            */
           close(serverSocket);
           handleClient(clientSocket, campusServerID);
           exit(0);
        } else{
            close(clientSocket);
        }

        cout << "Client connected\n";
    }
    // close server
    close(serverSocket);

    return 0;
}