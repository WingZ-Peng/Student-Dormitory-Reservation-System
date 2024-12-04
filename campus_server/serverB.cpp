#include "server.h"
using namespace std;

#define CAMPUS_SERVER_PORT_B 32778
#define MAIN_SERVER_PORT 34778
#define BUFFER_SIZE 1024

class CampusServerB {
private:
    vector<string> departments_;
    unordered_map<string, int> room_availability_;
    unordered_map<string, int> availability_count_;
    unordered_map<string, int> building_ids_price_;
    unordered_map<string, vector<string>> building_ids_;

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

            building_ids_price_[building_id] = price;
            availability_count_[type] += availability;
            building_ids_[type].emplace_back(building_id);
            room_availability_[building_id] = availability;
        }
        file.close();
    }

    string handleAvailabilityRequest(const string& type) {
        cout << "Server B has received a query of Availability for room type " << type << endl;
        string response;
        string on_screen_msg;

        // If this room type cannot be found
        if (availability_count_.find(type) == availability_count_.end()) {
            response = "Not able to find the room type";
            on_screen_msg = "Room type " + type + " does not show up in Server B";
        }
        else if (availability_count_[type] == 0) {
            response = "The requested room is not available.";
            on_screen_msg = "Room type " + type + " does not available in Server B";
        }
        else {
            response = "Campus B found " + to_string(availability_count_[type]) 
                + " available rooms in " + type + " type dormitory. "
                + "Their Building IDs are: ";
            on_screen_msg = "Server B found totally: " + to_string(availability_count_[type]) 
                + " available rooms for " + type + " type dormitory in Building: ";
            for (const auto& id : building_ids_[type]) {
                response += id + ", ";
                on_screen_msg += id + ", ";
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
        cout << "Server B has received a query of Price for room type " << type << endl;
        string response;
        string on_screen_msg;

        // If this room type cannot be found
        if (building_ids_.find(type) == building_ids_.end()) {
            on_screen_msg = "Room type " + type + " does not show up in Server B";
            response = "Not able to find the room type";
        }
        else {
            // Get the vector of building IDs for the given type
            const vector<string>& type_building_ids_ = building_ids_[type];
            // Create a vector to store pairs of building IDs and prices
            vector<pair<string, int> > buildings_with_prices_;

            // Populate the vector with building IDs and corresponding prices
            for (const auto& building_id : type_building_ids_) {
                if (building_ids_price_.find(building_id) != building_ids_price_.end()) {
                    buildings_with_prices_.emplace_back(building_id, building_ids_price_[building_id]);
                }
            }

            // Sort the vector by price in non-decreasing order
            sort(buildings_with_prices_.begin(), buildings_with_prices_.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second < b.second;
            });

            response = "B found room type " + type + " with prices: \n";
            for (int i = 0; i < buildings_with_prices_.size(); ++i) {
                const auto& building = buildings_with_prices_[i];
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

        cout << "Server B has received a query of Reserve for room type "
            << type << " at Building ID " << building_id << endl; 

        if (building_ids_.find(type) == building_ids_.end()) {
            on_screen_msg = "Room type " + type + " does not show up in Server B";
            response = "Reservation failed: Not able to find the room type.";
        }
        else if (find(building_ids_[type].begin(), building_ids_[type].end(), building_id) == building_ids_[type].end()) {
            on_screen_msg = "Building ID " + building_id + " does not show up in Server B";
            response = "Reservation failed: Building ID " + building_id + " does not exist.";
        }
        else if (find(building_ids_[type].begin(), building_ids_[type].end(), building_id) != building_ids_[type].end() 
                && room_availability_[building_id] == 0) {
            on_screen_msg = "Server B found room type " + type + " in Building ID " + building_id 
                + '.' + '\n' + "This room is not available.";
            response = "Reservation failed: Building ID " + building_id + " room type " + type
                + " is not available.";
        }
        else {
            on_screen_msg = "Server B found room type " + type + " in Building ID " + building_id 
                + '.' + '\n' + "This room availability is " + to_string(room_availability_[building_id])
                + '.' + '\n';
            // update
            room_availability_[building_id] -= 1;
            on_screen_msg += "This room is reserved, and availability is updated to " 
                + to_string(room_availability_[building_id]) + '.';
            response = "Reservation is successful for Campus B Building ID " + building_id + '!';
        }
        cout << on_screen_msg << endl;

        return response;
    }

public:
    CampusServerB(const string& file_path) {
        readData(file_path);
    }

    void start() {
        int sockfd;
        char buffer[BUFFER_SIZE];
        sockaddr_in server_address, main_server_address;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1) {
            cerr << "Socket creation failed" << endl;
            exit(1);
        }

        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(CAMPUS_SERVER_PORT_B);
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
        string response = "B";
        for (const auto& department : departments_) {
            response += "," + department;
        }
        sendto(sockfd, response.c_str(), response.size(), 0, (const sockaddr*)&main_server_address, len);
        cout << "Server B has sent a department list to Main Server" << endl << endl;

        // Stand by for further queries
        while (true) {
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
            cout << "Server B has sent the results to Main Server" << endl << endl;
        }

        close(sockfd);
    }
};

int main() {
    string file_path = "../data/dataB.txt";
    CampusServerB server(file_path);

    cout << "The Server B is up and running using UDP on port " << CAMPUS_SERVER_PORT_B << endl;
    server.start();

    return 0;
}