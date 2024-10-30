#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;
namespace Utility {
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
}