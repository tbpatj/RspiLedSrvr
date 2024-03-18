#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <memory> // Include the <memory> header for std::make_unique
#include <opencv2/opencv.hpp>
#include "./resources/json.hpp"
using json = nlohmann::json;
#include "./resources/httplib.h"


//Debug varaibles
const bool show_processed_image = true;
const bool show_webcam_feed = false;
const bool show_LEDS = true;
const bool show_animation = true;
const bool write_frame_proccessor_data = false;

#include "./src/utils/utils.cpp"

#include "./src/device/captureDevice/capture-device.cpp"
CaptureDevice captureDevice;

#include "./src/animations/animation.cpp"
std::vector<Animation> animations;
#include "./src/animations/init-animations.cpp"

#include "./src/presets/preset.cpp"
std::vector<Preset> presets;

//Actual code
#include "./src/device/device.cpp"
#include "./src/device/ledDevice/addressableLedDevice.cpp"

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
    initAnimations();
    loadDeviceAndPresets();
    std::thread serverThread(RunLedServer);
     //inital content loading
    while(running == 1){
        //run the capture device
        if(captureDevice.isCapturing){
            captureDevice.updateFrame();
            captureDevice.processFrame();
            // if( cv::waitKey (30) >= 0) {
            //     std::cout << "Closing" << std::endl;
            //     break;
            // };
        }
        //run the led devices
        for (int i = 0; i < devices.size(); i++) {
            devices[i]->update();
        }
    }
    saveDevices();
    savePresets();
    serverThread.join();
    return 0;
}