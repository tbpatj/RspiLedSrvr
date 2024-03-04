#include <fstream>
#include <string>
#include <vector>
#include <memory> // Include the <memory> header for std::make_unique
#include <opencv2/opencv.hpp>
#include "./resources/json.hpp"
using json = nlohmann::json;
#include "./resources/httplib.h"

//Debug varaibles
const bool show_processed_image = true;
const bool show_webcam_feed = false;
const bool write_frame_proccessor_data = true;

#include "./src/utils/utils.cpp"

#include "./src/device/captureDevice/capture-device.cpp"
CaptureDevice captureDevice;

//Actual code
#include "./src/device/device.cpp"
#include "./src/device/ledDevice/ledDevice.cpp"

// global declaration of devices
std::vector<std::unique_ptr<Device>> devices;
int running = 1;

// Load and Save Scripts
#include "./src/files/load-save.cpp"

// global declaration of server
httplib::Server svr;
#include "./src/server/server.cpp"

//TODO
//make it so the LEDS are controlled by a sort of image, the image if it is too small can have a interpolation between pixel values to make it bigger.


int main(){

    //inital content loading
    loadDevices();

    std::thread serverThread(RunLedServer);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while(running == 1){
        if(captureDevice.isCapturing){
            captureDevice.updateFrame();
            captureDevice.processFrame();
            // if( cv::waitKey (30) >= 0) {
            //     std::cout << "Closing" << std::endl;
            //     break;
            // };
            
        }
        for (int i = 0; i < devices.size(); i++) {
            devices[i]->update();
        }
    }
    saveDevices();
    serverThread.join();
    return 0;
}