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

        LedController* ledController; // Declare a pointer to the abstract class

        //TV stuff
        std::vector<int> leds;
        std::vector<int> leds2;

        //transition variables
        TransitionObject t;

        //animation variables
        AnimationObject a;

        //editor transition
        TransitionObject editorT;

        //testing purposes
        cv::Mat ledImage;

        int editIndex = -1;

public:


    void update() override {
        updateTiming();
        if(settings.power){
            //-1 is the tv mode
            if(editIndex >= 0){
                if(settings.mode == -1){
                    using_webcam = true;
                    a.setAnimImage(captureDevice.getMappingsImage());
                    a.setAnimIndx(0);
                    updateFromImageAnimation();
                }
                for(int i = 0; i < ledCount; i++){
                    if(i >= settings.mappings[editIndex].ledSIndx && i <= settings.mappings[editIndex].ledEIndx){
                        if(settings.mode != -1){
                            updateLED(i, 255, 255, 255);
                        }
                        if(t.getPercT() >= 1) leds2[i] = leds[i];
                    } else {
                        updateLED(i, 0, 0, 0);
                        if(t.getPercT() >= 1) leds2[i] = leds[i];
                    }
                }
                
                //if we have transitioned long enough then move on
                editorT.updateTiming();
                if(editorT.getPercT() >= 1.0f ){
                    editorT.setPercT(1.0f);
                    editIndex = -1;
                    t.resetTiming();
                }
            } else if(editIndex == -2) { //editing length mappings, so display the length of leds
                 for(int i = 0; i < ledCount; i++){
                    updateLED(i, 255, 255, 255);
                    if(t.getPercT() >= 1) leds2[i] = (255 << 16) | (255 << 8) | (255);
                 }
            } else if(editIndex == -1){
                //-1 is tv mode so it's set to match the tv
                if(settings.mode == -1){
                    using_webcam = true;
                    a.setAnimImage(captureDevice.getImage());
                    //make it so it doesn't try to go to the next "frame" of the tv image, which just updates the same frame every time
                    a.setAnimIndx(0);
                    updateFromImageAnimation();
                }else {
                    updateFromImageAnimation();
                }
            }
            ledController->render();
        } else if(t.getPercT() < 2.0f){
            //this block of code is in case we turn the power off on the leds we want to transition to power off and make sure they are turned off all the way
            if(t.getPercT() >= 1) t.setPercT(1.0f);  //set the transition percentage to 1 so we don't interpolate between some other value
            for(int i = 0; i < ledCount; i++){
                updateLED(i, 0, 0, 0);
            }
            if(t.getPercT() >= 1) t.setPercT(3.0f); //if we did reach full power down, make sure to set the percentage to 3 so we don't keep updating the leds in the future
            ledController->render();
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

    int usingTV() override {
        return settings.mode == -1;
    }
    
    void setPreset(std::string presetName) override {
        preset = presetName;
    }

    void updateTiming() {
        t.updateTiming();
        if(settings.mode != -1 && settings.power) {
            //update animation timing
            a.updateTiming(settings.animSpeed);
        }
    }

    void resetTransition() {
        t.resetTiming();
        //clear out the leds2 then copy leds into it so then we can transition between what was playing before and what is coming up.
        leds2.clear();
        for(int i = 0; i < leds.size(); i ++){
            leds2.push_back(leds[i]);
        }
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
                resetTransition();
                if(settings.mode >= 0 && settings.mode < animations.size() && oldMode != settings.mode){
                    a.resetTiming(settings.mode);
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

    void showLength() override {
        editorT.resetTiming();
        t.resetTiming();
        editIndex = -2;
    }

    void showMapping(int index) override {
        if(index < settings.mappings.size()){
            editorT.resetTiming();
            t.resetTiming();
            editIndex = index;
        } else {
            editIndex = -1;
        }
    }

    void setData(json data) override {
        bool updateSettings = true;
    
        //update transition object
        t.setData(data);
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
                a.resetTiming(settings.mode);
            }
            resetTransition();

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
                {"transition_speed", t.getTransitionSpeed()},
                // //depending on the type of the device the pinout will be different so change the response based on that.
                {"pin_out", pinOut},
            };
        return data;
    }

    AddressableLedDevice(std::string newName, int newType) {
        name = newName;
        t = TransitionObject();
        type = 1;
        pinOut = 0;
    }
    AddressableLedDevice(json data) {
        try{
            setName(data);
            setType(data);
            setData(data);
            std::cout << "Device: " << name << " type: " << "addressable" << std::endl;
            //initialize the transition object
            t = TransitionObject();
            editorT = TransitionObject(3000);
            //define the actual led controller that utilizes some library or other implementation to control the specific kind of light it is
            //currently type is 0 just due to no other type of addressable led being implemented
            if(type == 1){
                ledController = new WS2811xController(pinOut, ledCount);
            }
            
            if(show_LEDS) ledImage = cv::Mat::zeros(cv::Size(ledCount,1), CV_8UC3); // Update to use 3 color channels (CV_8UC3) instead of grayscale (0)
        }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            name = "default";
        }
    }

    //update the LED with a integer that stores r g and b values
    void updateLED(int index, int color){
        if(t.getPercT() < 1){
            int nColor = interpolate(leds2[index], color, t.getPercT());
            leds[index] = nColor;
            
        } else leds[index] = color;
        ledController->setLEDColor(index, leds[index]);
        //debug option to show the leds in an image
        if(show_LEDS) updateDebugPixel(index, leds[index]);
    }

    void updateLED(int index, int r, int g, int b){
        if(t.getPercT() < 1){
            int nColor = interpolate(leds2[index], r, g, b, t.getPercT());
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

    //here we will take an animation made up of an image, each row is a set of the current frame of the animation.
    //each column is a pixel or led in the animation. If there aren't enough to map to each led in the set then we interpolate the mapped leds.
    //we'll want to change it so we can map led's to indecies in an animation. We could potentially take the tv settings out, then we can have the mapped leds be a part of presets and such.
    // then move that into a new setting where we can map the leds to the animation indecies, and even redo the tv animation for that specific purpose.

    //we'll want to look into encoding an image with data in the background. So then potentially we could use the last couple of pixels on a row to encode other data, such as time till next frame, or if it interpolates or not.

    //maybe later make it so the images can be modified while in use so like it color shifts. So if you want to have a breathing animation. It's 2 rows, a colored pixel and a non colored. Then after it completes each animation it shifts the colors by using a filter
    void updateFromImageAnimation(){
        //get current frame interpolated and stuff
        cv::Mat frame = a.getCurFrame();
        cv::Vec3b* row = frame.ptr<cv::Vec3b>(0);
        if(a.getCCols() > 0){
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
            int mappingSize = settings.mappings.size();
            int mappingIStart = 0;
            if(editIndex >= 0){
                mappingSize = editIndex + 1;
                mappingIStart = editIndex;
            }

            for (int i = mappingIStart; i < mappingSize; i++) {
                start = settings.mappings[i].ledSIndx;
                end = settings.mappings[i].ledEIndx;
                iterations = min(std::abs(settings.mappings[i].mapEIndx - settings.mappings[i].mapSIndx),a.getCCols());
                startMapIndx = min(settings.mappings[i].mapEIndx, settings.mappings[i].mapSIndx);
                //set up the loop values so we go in the correct direction
                length = min(end, ledCount) - min(start,ledCount);
                if(length < 0){
                    increment = -1;
                    //since length is less than 0 we need to negate it. techically an abs function
                    length = min(std::abs(length) + 1, ledCount);
                    startJ = length - 1;
                    offsetI = end;
                } else {
                    increment = 1;
                    startJ = 0;
                    offsetI = start;
                    length = min(length + 1, ledCount);
                }
                step = static_cast<float>(iterations) / (length == 0 ? 1.0f : length);
                if(step < 1){ //we'll need to interpolate between two pixels
                    // //initialize variable declarations before looping
                    float rowIndex = 0.0f;
                    int indx1 = 0;
                    int indx2 = 0;
                    float perc = 0.0f;
                    int nColor = 0;
                    int stepI = 0;
                    for(int j = startJ; j >= 0 && j <= length && j + offsetI < ledCount; j = j + increment){
                        //get the indecies that we'll interpolate between
                        rowIndex = static_cast<float>(stepI) * step;
                        indx1 = static_cast<int>(std::floor(rowIndex));
                        indx2 = static_cast<int>(std::ceil(rowIndex)) + startMapIndx;
                        //get the fraction of how far we are to the next index so we can interpolate properly
                        perc = rowIndex - static_cast<float>(indx1);
                        indx1 += startMapIndx;
                        if(indx1 > a.getCCols() - 1) indx1 = 0;
                        if(indx2 > a.getCCols() - 1) indx2 = indx1;
                        cv::Vec3b pixel1 = row[indx1];
                        cv::Vec3b pixel2 = row[indx2];
                        //perform the interpolation
                        nColor = interpolate(pixel1[2], pixel1[1], pixel1[0], pixel2[2], pixel2[1], pixel2[0], perc);
                        updateLED(j + offsetI, nColor);
                        //update the step counter
                        stepI ++;
                    }
                } else {
                    int stepI = 0;
                    //if the step is greater than 1 then that means no interpolation is required
                    //make sure increment is going in correct direction
                    for(int j = startJ; j >= 0 && j < length; j = j + increment){
                        int rowI = static_cast<int>(std::round(stepI * step)) + startMapIndx;
                        if(rowI > a.getCCols() - 1) rowI = 0;
                        cv::Vec3b pixel = row[rowI];
                        // std::cout << j << " rowI: " << rowI << " colors: R: " << pixel[2] << " G: " << pixel[1] << " B: " << pixel[0] << std::endl;
                        updateLED(j + offsetI, pixel[2], pixel[1], pixel[0]);
                        stepI ++;
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
