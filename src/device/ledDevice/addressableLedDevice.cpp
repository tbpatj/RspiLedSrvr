#include "./pins.cpp"
#include "./settings.cpp"
#include "./ledControllers/ledControllers.cpp"

class AddressableLedDevice : public Device {
private:
        //types are
        //0 for non-addressable LED
        //1 for addressable LED
        //we initialize the mode parameter with -1000 so we know it hasn't been intitalized, since 0 and -1 are used
        LedDeviceSettings settings = LedDeviceSettings("default", 0, -1000, false);
        int pinOut;
        int ledCount;
        std::string preset = "default";

        LedController* ledController; // Declare a pointer to the abstract class

        //TV stuff
        std::vector<int> leds;
        std::vector<int> leds2;

        //transition variables used for when a preset changes
        std::chrono::milliseconds tStart;
        float t = 0.0f;
        long long transitionSpeed = 1000;

        //animation variables
        int animIndx = 0;
        std::chrono::milliseconds aStart;
        float animT = 0.0f;
        cv::Mat animImage;

        //testing purposes
        cv::Mat ledImage;

public:


    void update() override {
        updateAnimationTiming();
        if(settings.power){
            //-1 is the tv mode
            if(settings.mode == -1){
                animImage = captureDevice.getImage();
                animIndx = 0;
                updateFromImageAnimation();
                // updateLedTVStyle();
            }else {
                updateFromImageAnimation();
            }
        } else {
            if(t < 1.0f){
                for(int i = 0; i < ledCount; i++){
                    updateLED(i, 0, 0, 0);
                }
            } else if(t < 2){
                //just make sure they are turned off all the waqy
                t=1;
                for(int i = 0; i < ledCount; i++){
                    updateLED(i, 0, 0, 0);
                }
                t=3;
            }
            if(t < 2){
                ledController->render();
            }
        }
        //debug options
        if(show_LEDS){
            if(ledCount > 0){
                cv::imshow(name, ledImage);
                if( cv::waitKey (30) >= 0) {
                    std::cout << "Closing" << std::endl;
                    return;
                };
            }
        }
    }
    
    void setPreset(std::string presetName) override {
        preset = presetName;
    }

    void updateAnimationTiming() {
        if(t < 1.0f){
            std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            t = static_cast<float>((now - tStart).count()) / transitionSpeed;
            // t = curTime.count() - tStart;
            if(t >= 1.0f) t = 1.0f;
        }
        if(settings.mode != -1 && settings.power) {
            //update animation timing
            std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            animT = static_cast<float>((now - aStart).count()) / settings.animSpeed;
            
            //if animation should move to next frame move it.
            if(animT >= 1){
                //reset the start time
                animT = 1.0f - min(animT,2.0f);
                // aStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                std::chrono::milliseconds animSpeedMs(static_cast<long long>(settings.animSpeed * animT));
                aStart = now - animSpeedMs;

                //make sure to add an if to check if it should move to the next frame or if it needs to repeat

                //move the current frame of the animation to the next
                animIndx++;
                if(animIndx >= animImage.rows) {
                    updateAnimationAfterLooped();
                    animIndx = 0;
                }
            }
        }
    }

    void resetTransitionTiming() {
         //reset timer for transitions... probably could be moved somewhere else
        t = 0;
        // tStart = std::chrono::high_resolution_clock::now();
        tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        //set the old leds for a transition
        leds2.clear();
        for(int i = 0; i < leds.size(); i ++){
            leds2.push_back(leds[i]);
        }
    }

    void resetAnimationTiming() {
        animImage = animations[settings.mode].getAnimation();
        //if the new animation that is loaded up is smaller than the last then we will need to move the current frame potentially.
        if(animIndx >= animImage.rows) {
            animT = 0;
            std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            // aStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            aStart = now;
            animIndx = 0;
        }
    }


    //here is where we will perform certain operations on the image of the animation to make it so the animation changes or something
    void updateAnimationAfterLooped() {
        // Call the colorShift function with the desired shift value
        // colorShift(animImage, 30); // Shift the hue by 30 degrees 
    }

    void colorShift(cv::Mat& image, int shift) {
        cv::Mat hsvImage;
        cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

        // Shift the hue channel
        hsvImage.forEach<cv::Vec3b>([shift](cv::Vec3b& pixel, const int* position) {
            pixel[0] = (pixel[0] + shift) % 180; // Wrap around the hue values
        });

        cv::cvtColor(hsvImage, image, cv::COLOR_HSV2BGR);
    }

    void setLEDCount(int count) {
        ledCount = count;
        leds.resize(ledCount);
        leds2.resize(ledCount);
    }

    void updateSettingsToPreset(std::string newPreset) {
        for(int i = 0; i < presets.size(); i++){
            if(presets[i].getName() == newPreset && presets[i].getNumType() == type && name == presets[i].getDeviceName()){
                int oldMode = settings.mode;
                preset = newPreset;
                settings.setData(presets[i].getJson());
                resetTransitionTiming();
                if(settings.mode >= 0 && settings.mode < animations.size() && oldMode != settings.mode){
                    resetAnimationTiming();
                }
                break;
            }
        }
    }

    void setType(json data){
        if(!data["type"].is_null()){
            //currently the default is ws2811
            type = 1;
            if(data["type"] == "addressable"){
                type = 1;
            }
        }
    }

    void setName(json data){
        //if there is a name attribute in the json then update the name attribute, if not then keep it as the name it was prior unless empty then set it to default
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
    }

    void setData(json data) override {
        bool updateSettings = true;
    
        //update timings
        transitionSpeed = data["transition_speed"].is_null() ? transitionSpeed : static_cast<long long>(data["transition_speed"]);
        //set up the preset
        if(!data["preset"].is_null()){
            std::string newPreset = static_cast<std::string>(data["preset"]);
            if(newPreset != preset && newPreset != "custom") {
                updateSettings = false;
                updateSettingsToPreset(newPreset);
            } else if(newPreset == "custom") {
                preset = newPreset;
            }
        }
        //led count
        setLEDCount(data["led_count"].is_null() ? ledCount : static_cast<int>(data["led_count"]));
        //set up the settings object
        if (!data["settings"].is_null() && updateSettings) {
            //get the mode before a potential update so we can check if we actually need to change the animation image and timings
            int oldMode = settings.mode;
            settings.setData(data["settings"]);
            //if the mode isn't -1 then we want to load up the animation image if it was changed
            if(!data["settings"]["mode"].is_null() && settings.mode >= 0 && settings.mode < animations.size() && oldMode != settings.mode){
                resetAnimationTiming();
            }
            resetTransitionTiming();

        }
        pinOut = data["pin_out"].is_null() ? pinOut : static_cast<int>(data["pin_out"]);
    }

    json getJson() override {
        // implementation for getting json on a LedDevice
        json data = {
                {"name", name}, // Add a new key-value pair to the JSON object
                {"type", "addressable"}, // Add another key-value pair named "response"
                {"settings", settings.getJson()},
                {"led_count", ledCount},
                {"preset", preset},
                {"transition_speed", transitionSpeed},
                // //depending on the type of the device the pinout will be different so change the response based on that.
                {"pin_out", pinOut},
            };
        return data;
    }

    AddressableLedDevice(std::string newName, int newType) {
        name = newName;
        t = 0.0f;
        type = 1;
        tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        // settings = new LedDeviceSettings("default", 0, "default", false);
        pinOut = 0;
    }
    AddressableLedDevice(json data) {
        try{
            setName(data);
            setType(data);
            setData(data);
            std::cout << "Device: " << name << " type: " << "addressable" << std::endl;
            t = 0.0f;
            //define the actual led controller that utilizes some library or other implementation to control the specific kind of light it is
            //currently type is 0 just due to no other type of addressable led being implemented
            if(type == 1){
                ledController = new WS2811xController(pinOut, ledCount);
            }
            tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            if(show_LEDS) ledImage = cv::Mat::zeros(cv::Size(ledCount,1), CV_8UC3); // Update to use 3 color channels (CV_8UC3) instead of grayscale (0)
        }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            name = "default";
        }
    }

    //update the LED with a integer that stores r g and b values
    void updateLED(int index, int color){
        if(t < 1){
            int nColor = interpolate(leds2[index], color, t);
            leds[index] = nColor;
            
        } else leds[index] = color;
        ledController->setLEDColor(index, leds[index]);
        //debug option to show the leds in an image
        if(show_LEDS) updateDebugPixel(index, leds[index]);
    }

    void updateLED(int index, int r, int g, int b){
        if(t < 1){
            int nColor = interpolate(leds2[index], r, g, b, t);
            leds[index] = nColor;
            
        } else leds[index] = (r << 16) | (g << 8) | (b);
        ledController->setLEDColor(index, leds[index]);
        //debug option to show the leds in an image
        if(show_LEDS) updateDebugPixel(index, leds[index]);
    }

    int interpolate(int color, int r2, int g2, int b2, float fraction){
        unsigned char   r1 = (color >> 16) & 0xff;
        unsigned char   g1 = (color >> 8) & 0xff;
        unsigned char   b1 = color & 0xff;

         return (int) ((r2 - r1) * fraction + r1) << 16 |
                    (int) ((g2 - g1) * fraction + g1) << 8 |
                    (int) ((b2 - b1) * fraction + b1);
    }

    int interpolate(int r1, int g1, int b1, int r2, int g2, int b2, float fraction){
         return (int) ((r2 - r1) * fraction + r1) << 16 |
                    (int) ((g2 - g1) * fraction + g1) << 8 |
                    (int) ((b2 - b1) * fraction + b1);
    }

    int interpolate(int color1, int color2, float fraction)
    {
            //bit shift the colors back
            unsigned char   r1 = (color1 >> 16) & 0xff;
            unsigned char   r2 = (color2 >> 16) & 0xff;
            unsigned char   g1 = (color1 >> 8) & 0xff;
            unsigned char   g2 = (color2 >> 8) & 0xff;
            unsigned char   b1 = color1 & 0xff;
            unsigned char   b2 = color2 & 0xff;

            //interpolate between the colors
            return (int) ((r2 - r1) * fraction + r1) << 16 |
                    (int) ((g2 - g1) * fraction + g1) << 8 |
                    (int) ((b2 - b1) * fraction + b1);
    }

    cv::Mat interpolateFrames(const cv::Mat& curFrame, const cv::Mat& nextFrame, float fraction) {
        cv::Mat interpolatedFrame;
        cv::addWeighted(curFrame, 1 - fraction, nextFrame, fraction, 0, interpolatedFrame);
        return interpolatedFrame;
    }

    //here we will take an animation made up of an image, each row is a set of the current frame of the animation.
    //each column is a pixel or led in the animation. If there aren't enough to map to each led in the set then we interpolate the mapped leds.
    //we'll want to change it so we can map led's to indecies in an animation. We could potentially take the tv settings out, then we can have the mapped leds be a part of presets and such.
    // then move that into a new setting where we can map the leds to the animation indecies, and even redo the tv animation for that specific purpose.

    //we'll want to look into encoding an image with data in the background. So then potentially we could use the last couple of pixels on a row to encode other data, such as time till next frame, or if it interpolates or not.

    //maybe later make it so the images can be modified while in use so like it color shifts. So if you want to have a breathing animation. It's 2 rows, a colored pixel and a non colored. Then after it completes each animation it shifts the colors by using a filter
    void updateFromImageAnimation(){
        //get current frame
        if(animImage.cols > 0 && animImage.rows > 0 && animIndx < animImage.rows){
            cv::Rect roi(0,animIndx,animImage.cols,1);
            cv::Mat cFrame = animImage(roi);
            //get next frame
            int ceilFrameIndx = animIndx + 1;
            if(ceilFrameIndx >= animImage.rows) ceilFrameIndx = 0;
            //if the next frame is different than the current frame then interpolate between the two frames
            if(ceilFrameIndx != animIndx){
                cv::Rect roi2(0,ceilFrameIndx,animImage.cols,1);
                cv::Mat ceilFrame = animImage(roi2);

                //interpolate between the two frames so we are at the current frame
                cFrame = interpolateFrames(cFrame, ceilFrame, animT);
            }
            //get a pointer to the row of the current frame
            cv::Vec3b* row = cFrame.ptr<cv::Vec3b>(0);
            if(show_animation) cv::imshow("Animation", cFrame);
            
            //set up the start of the mapping loops
            int start = 0;
            int end = 0;
            int startMapIndx = 0;
            int length = 0;
            int increment = 1;
            int startJ = 0;
            int iterations = 0;
            int offsetI = 0;
            float step = 1.0f;
            //proobably test if the image is loaded
            if(1 == 1){
                for (int i = 0; i < settings.mappings.size(); i++) {
                    start = settings.mappings[i].ledSIndx;
                    end = settings.mappings[i].ledEIndx;
                    iterations = min(std::abs(settings.mappings[i].mapEIndx - settings.mappings[i].mapSIndx),cFrame.cols);
                    startMapIndx = min(settings.mappings[i].mapEIndx, settings.mappings[i].mapSIndx);
                    //set up the loop values so we go in the correct direction
                    length = min(end, ledCount) - min(start,ledCount);
                    if(length < 0){
                        increment = -1;
                        //since length is less than 0 we need to negate it. techically an abs function
                        length = std::abs(length);
                        startJ = length - 1;
                        offsetI = end;
                    } else {
                        increment = 1;
                        startJ = 0;
                        offsetI = start;
                    }
                    step = static_cast<float>(iterations) / (length == 0 ? 1.0f : length);
                    if(step < 1){
                        // //initialize variable declarations before looping
                        float rowIndex = 0.0f;
                        int indx1 = 0;
                        int indx2 = 0;
                        float perc = 0.0f;
                        int nColor = 0;
                        int stepI = 0;
                        for(int j = startJ; j >= 0 && j < length && j + offsetI < ledCount; j = j + increment){
                            //we'll need to interpolate between two pixels

                            //get the indecies that we'll interpolate between
                            rowIndex = static_cast<float>(stepI) * step;
                            indx1 = static_cast<int>(std::floor(rowIndex));
                            indx2 = static_cast<int>(std::ceil(rowIndex)) + startMapIndx;
                        //     std::cout << " j: " << j << "rowIndex is: " << rowIndex << " index1: " << indx1 << " index2: " << indx2 << std::endl;
                            //get the fraction of how far we are to the next index so we can interpolate properly
                            perc = rowIndex - static_cast<float>(indx1);
                            indx1 += startMapIndx;
                            if(indx1 > cFrame.cols - 1) indx1 = 0;
                            if(indx2 > cFrame.cols - 1) indx2 = indx1;
                            cv::Vec3b pixel1 = row[indx1];
                            cv::Vec3b pixel2 = row[indx2];
                            //perform the interpolation
                            nColor = interpolate(pixel1[2], pixel1[1], pixel1[0], pixel2[2], pixel2[1], pixel2[0], perc);
                            updateLED(j + offsetI, nColor);
                            //update the step counter
                            stepI ++;
                        }
                        // std::cout << "step is: " << step << " iterations: " << iterations << " length: " << length << std::endl;
                    } else {
                        int stepI = 0;
                        //if the step is greater than 1 then that means no interpolation is required
                        //make sure increment is going in correct direction
                        for(int j = startJ; j >= 0 && j < length; j = j + increment){
                            int rowI = static_cast<int>(std::round(stepI * step)) + startMapIndx;
                            if(rowI > cFrame.cols - 1) rowI = 0;
                            cv::Vec3b pixel = row[rowI];
                            // std::cout << j << " rowI: " << rowI << " colors: R: " << pixel[2] << " G: " << pixel[1] << " B: " << pixel[0] << std::endl;
                            updateLED(j + offsetI, pixel[2], pixel[1], pixel[0]);
                            stepI ++;
                        }
                    }
                }
            }
        }
    }

    //------- DEBUG ------
    void updateDebugPixel(int index, int color){
        cv::Vec3b& pixel = ledImage.at<cv::Vec3b>(0,index);
        int r = static_cast<int>((leds[index] >> 16) & 0xff);
        int g =  static_cast<int>((leds[index] >> 8) & 0xff);
        int b =  static_cast<int>(leds[index] & 0xff);
        pixel[0] = b; // Blue channel
        pixel[1] = g; // Green channel
        pixel[2] = r; // Red channel
    }
};