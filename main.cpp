#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
// #include <pigpio.h>
#include <memory> // Include the <memory> header for std::make_unique
#include <opencv2/opencv.hpp>
#include "./resources/json.hpp"
using json = nlohmann::json;
#include "./resources/httplib.h"
#include "./src/utils/base-64.cpp"


//Debug varaibles
const bool show_processed_image = true;
const bool show_webcam_feed = false;
const bool show_LEDS = true;
const bool show_animation = true;
const bool write_frame_proccessor_data = false;

//Capture device variable
bool using_webcam = false;
bool tv_sleeping = false;
bool tv_no_signal = false;

//GPIO variables
bool gpio_initialised = false;

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
#include "./src/device/ledDevice/ledDevices.cpp"

// global declaration of devices
std::vector<std::unique_ptr<Device>> devices;
int running = 1;

//load in the sleep controller
#include "./src/sleeping/sleepController.cpp"
SleepController sleepController;

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

    // if(gpioInitialise() < 0){
    //     gpio_initialised = false;
    //     std::cout << "Failed to initialise GPIO" << std::endl;
    // } else gpio_initialised = true;

    std::thread serverThread(RunLedServer);

    auto nextFrame = std::chrono::steady_clock::now();
     //inital content loading
    while(running == 1){
        nextFrame += std::chrono::milliseconds(1000 / 60);
        //run the capture device
        if(captureDevice.isCapturing && using_webcam){
            captureDevice.updateFrame();
            captureDevice.processFrame();
            // if( cv::waitKey (30) >= 0) {
            //     std::cout << "Closing" << std::endl;
            //     break;
            // };
        }
        //debug for the processed image
        if(show_processed_image) cv::imshow("New Image", captureDevice.getImage());
        //debug for the webcam feed
        if(show_webcam_feed) cv::imshow("Webcam", captureDevice.getRawFrame());
        sleepController.update();
        using_webcam = false;
        //run the led devices
        for (int i = 0; i < devices.size(); i++) {
            devices[i]->update();
        }
        std::this_thread::sleep_until(nextFrame);
    }
    saveDevices();
    savePresets();
    // terminate the gpio library
    // gpioTerminate();
    serverThread.join();
    return 0;
}