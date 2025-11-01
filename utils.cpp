#include <iostream>
#include <cstdlib>
using namespace std;

namespace zbs {
    void error(string msg) {
        cout << "[ERROR]: " << msg << '\n';
        exit(1);
    }
    
    void log(string msg) {
        cout << "[INFO]: " << msg<< '\n';
    }
}
