#include <fstream>
#include <string>
#include <vector>
#include <memory> // Include the <memory> header for std::make_unique
#include "./resources/json.hpp"
using json = nlohmann::json;
#include "./resources/httplib.h"

//Actual code
#include "./src/device/device.cpp"
#include "./src/device/ledDevice/ledDevice.cpp"

// global declaration of devices
std::vector<std::unique_ptr<Device>> devices;

// global declaration of server
httplib::Server svr;
#include "./src/server/server.cpp"

int main(){
    RunLedServer();
    // std::thread serverThread(RunLedServer);
    // std::ifstream f("example.json");
    // json data = json::parse(f);
    return 0;
}