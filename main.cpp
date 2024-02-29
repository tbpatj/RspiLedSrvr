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


// global declaration of variables
httplib::Server svr;
#include "./src/server/server.cpp"

int main(){
    std::vector<std::unique_ptr<Device>> devices;

    devices.push_back(std::make_unique<LedDevice>("Led1", 1));
    //start the server on another thread
    std::cout << "Starting server" << std::endl;

    devices[0]->update();
    RunLedServer();
    // std::thread serverThread(RunLedServer);
    // std::ifstream f("example.json");
    // json data = json::parse(f);
    return 0;
}