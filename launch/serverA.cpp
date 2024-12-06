#include "server.h"
using namespace std;

#define CAMPUS_SERVER_PORT_A 31778
#define MAIN_SERVER_PORT 34778
#define BUFFER_SIZE 1024

class CampusServerA {
private:
    struct RoomInfo {
        int available;
        int price;
    };

    vector<string> departments_;
    unordered_map<string, int> availability_count_;
    // type: {building ID: {availability, price}}
    unordered_map<string, unordered_map<string, RoomInfo> > rooms_database_;

    void readData(const string& file_path) {
        ifstream file(file_path);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << file_path << endl;
            return;
        }

        string line;
        // Read the first line (departments)
        if (getline(file, line)) {
            stringstream ss(line);
            string department;
            while (getline(ss, department, ',')) {
                departments_.emplace_back(Utility::trim(department));
            }
        }

        // Read the remaining lines (availability and building IDs)
        while (getline(file, line)) {
            stringstream ss(line);
            string type, building_id, availability_str, price_str;
            getline(ss, type, ',');
            getline(ss, building_id, ',');
            getline(ss, availability_str, ',');
            getline(ss,price_str, ',');
            
            int price = stoi(price_str);
            int availability = stoi(availability_str);

            availability_count_[type] += availability;
            RoomInfo room_info_ = RoomInfo{availability, price};
            rooms_database_[type].emplace(building_id, room_info_);
        }
        file.close();
    }

    void print() const {
        cout << "----------------------------" << endl;
        for (const auto& pair : availability_count_) {
            cout << pair.first << " - " << pair.second << endl; 
        }
        cout << endl;
        cout << "----------------------------" << endl;
        for (const auto& type_pair : rooms_database_) {
            const string& type = type_pair.first;
            const auto& rooms = type_pair.second;

            for (const auto& room_pair : rooms) {
                string room_id = room_pair.first;
                const RoomInfo& info = room_pair.second;

                cout << type << "," << room_id << "," 
                    << info.available << "," << info.price << endl;
            }
        }
        cout << endl;
    }

    string handleAvailabilityRequest(const string& type) {
        cout << "Server A has received a query of Availability for room type " << type << endl;
        string response;
        string on_screen_msg;

        // If this room type cannot be found
        if (availability_count_.find(type) == availability_count_.end()) {
            response = "Not able to find the room type";
            on_screen_msg = "Room type " + type + " does not show up in Server A";
        }
        else {
            response = "Campus A found " + to_string(availability_count_[type]) 
                + " available rooms in " + type + " type dormitory. "
                + "Their Building IDs are: ";
            on_screen_msg = "Server A found totally: " + to_string(availability_count_[type]) 
                + " available rooms for " + type + " type dormitory in Building: ";

            for (const auto& room_pair : rooms_database_[type]) {
                response += room_pair.first + ", ";
                on_screen_msg += room_pair.first + ", ";
            }
            response.pop_back(); // Remove trailing comma
            response.pop_back(); // Remove trailing space
            on_screen_msg.pop_back(); // Remove trailing comma
            on_screen_msg.pop_back(); // Remove trailing space
            response += '.';
            on_screen_msg += '.';
        }
        cout << on_screen_msg << endl;

        return response;
    }

    string handlePriceRequest(const string& type) {
        cout << "Server A has received a query of Price for room type " << type << endl;
        string response;
        string on_screen_msg;

        // If this room type cannot be found
        if (availability_count_.find(type) == availability_count_.end()) {
            on_screen_msg = "Room type " + type + " does not show up in Server A";
            response = "Not able to find the room type";
        }
        else {
            // Get the vector of building IDs for the given type
            const unordered_map<string, RoomInfo>& type_pair = rooms_database_[type];
            // Create a vector to store pairs of building IDs and prices
            vector<pair<string, int> > room_id_with_price_;

            // Populate the vector with building IDs and corresponding prices
            for (const auto& room_pair : type_pair) {
                string room_id = room_pair.first;
                const RoomInfo& room_info = room_pair.second;
                room_id_with_price_.emplace_back(room_id, room_info.price);
            }

            // Sort the vector by price in non-decreasing order
            sort(room_id_with_price_.begin(), room_id_with_price_.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second < b.second;
            });

            response = "A found room type " + type + " with prices: \n";
            for (size_t i = 0; i < room_id_with_price_.size(); ++i) {
                const auto& building = room_id_with_price_[i];
                response += "Building ID " + building.first + ", Price $" + to_string(building.second) + '\n';
            }
            on_screen_msg = "Server " + response;
            response = "Campus " + response;
        }
        cout << on_screen_msg << endl;

        return response;
    }

    string handleReserveRequest(const string& query) {
        string response;
        string on_screen_msg;
        size_t comma_pos = query.find(',');
        string type = query.substr(0, comma_pos);
        string building_id = query.substr(comma_pos + 1);

        cout << "Server A has received a query of Reserve for room type "
            << type << " at Building ID " << building_id << endl; 

        if (availability_count_.find(type) == availability_count_.end()) {
            on_screen_msg = "Room type " + type + " does not show up in Server A";
            response = "Reservation failed: Not able to find the room type.";
        }
        else if (rooms_database_[type].find(building_id) == rooms_database_[type].end()) {
            on_screen_msg = "Building ID " + building_id + " does not show up in Server A";
            response = "Reservation failed: Building ID " + building_id + " does not exist.";
        }
        else if (rooms_database_[type].find(building_id) != rooms_database_[type].end()
                && rooms_database_[type][building_id].available == 0) {
            on_screen_msg = "Server A found room type " + type + " in Building ID " + building_id 
                + '.' + '\n' + "This room is not available.";
            response = "Reservation failed: Building ID " + building_id + " room type " + type
                + " is not available.";
        }
        else {
            on_screen_msg = "Server A found room type " + type + " in Building ID " + building_id 
                + '.' + '\n' + "This room availability is " + to_string(rooms_database_[type][building_id].available)
                + '.' + '\n';
            // update
            rooms_database_[type][building_id].available -= 1;
            availability_count_[type] -= 1;
            on_screen_msg += "This room is reserved, and availability is updated to " 
                + to_string(rooms_database_[type][building_id].available) + '.';
            response = "Reservation is successful for Campus A Building ID " + building_id + '!';
        }
        cout << on_screen_msg << endl;

        return response;
    }

public:
    CampusServerA(const string& file_path) {
        readData(file_path);
        // print();
    }

    void start() {
        int sockfd;
        char buffer[BUFFER_SIZE];
        sockaddr_in server_address, main_server_address;
        
        // Create socket by using UDP
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1) {
            cerr << "Socket creation failed" << endl;
            exit(1);
        }

        // Configure server address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(CAMPUS_SERVER_PORT_A);
        inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

        if (::bind(sockfd, (const sockaddr*)&server_address, sizeof(server_address)) < 0) {
            cerr << "Bind failed" << endl;
            close(sockfd);
            exit(1);
        }

        // Wait for wake-up message from Main Server
        socklen_t len = sizeof(main_server_address);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&main_server_address, &len);

        // Send department list to Main Server
        string response = "A";
        for (const auto& department : departments_) {
            response += "," + department;
        }
        sendto(sockfd, response.c_str(), response.size(), 0, (const sockaddr*)&main_server_address, len);
        cout << "Server A has sent a department list to Main Server" << endl << endl;

        // Stand by for further queries
        while (true) {
            // Clean buffer and be ready 
            memset(buffer, 0, sizeof(buffer));
            n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&main_server_address, &len);
            buffer[n] = '\0';

            string query(buffer);
            size_t comma_pos = query.find(',');
            string action = query.substr(0, comma_pos);
            query = query.substr(comma_pos + 1);
            
            if (action == "availability") {
                response = handleAvailabilityRequest(query);
            }
            else if (action == "price") {
                response = handlePriceRequest(query);
            }
            else if (action == "reserve") {
                response = handleReserveRequest(query);
            }

            sendto(sockfd, response.c_str(), response.size(), 0, (const sockaddr*)&main_server_address, len);

            memset(buffer, 0, sizeof(buffer));
            cout << "Server A has sent the results to Main Server" << endl;
            cout << endl;
        }

        close(sockfd);
    }
};

int main() {
    string file_path = "dataA.txt";
    CampusServerA server(file_path);

    cout << "The Server A is up and running using UDP on port " << CAMPUS_SERVER_PORT_A << endl;
    server.start();

    return 0;
}
