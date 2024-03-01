#include <fstream>
#include <string>
#include <vector>
#include <memory> // Include the <memory> header for std::make_unique
#include <opencv2/opencv.hpp>
#include "./resources/json.hpp"
using json = nlohmann::json;
#include "./resources/httplib.h"

#include "./src/device/captureDevice/capture-device.cpp"
CaptureDevice captureDevice;

//Actual code
#include "./src/device/device.cpp"
#include "./src/device/ledDevice/ledDevice.cpp"

// global declaration of devices
std::vector<std::unique_ptr<Device>> devices;

// Load and Save Scripts
#include "./src/files/load-save.cpp"

// global declaration of server
httplib::Server svr;
#include "./src/server/server.cpp"

int main(){

    //inital content loading
    loadDevices();

    std::thread serverThread(RunLedServer);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while(!captureDevice.isWindowClosed()){
        if(captureDevice.isCapturing){
            captureDevice.updateFrame();
            captureDevice.show();
            if( cv::waitKey (30) >= 0) break;
        }
    }
    saveDevices();
    serverThread.join();
    return 0;
}