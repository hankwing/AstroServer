#include <iostream>
#include <sstream>
#include <queue>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>

using namespace std;

int main() {
    string s;
    stringstream ss;
    while (true) {
        getline(cin, s);
        vector<string> fields;
        int num;
        float value;
        long long timeStamp;
        boost::split(fields, s, boost::is_any_of(" "));
        
        ss.clear();
        ss.str("");
        ss << fields[0];
        ss >> num;
        cout << num << endl;
        
        ss.clear();
        ss.str("");
        ss << fields[1];
        ss >> value;
        cout << value << endl;
        
        ss.clear();
        ss.str("");
        ss << fields[2];
        ss >> timeStamp;
        cout << timeStamp << endl;
    }
    return 0;
}

