class NonAddressableLedDevice : public Device {
private:
        //we initialize the mode parameter with -1000 so we know it hasn't been intitalized, since 0 and -1 are used
        LedDeviceSettings settings = LedDeviceSettings("default", 0, -1000, false);
        NonAddressableController* nac;

        std::string preset = "default";

        cv::Vec3i rgb = cv::Vec3i(0,0,0);

        cv::Vec3i rgb2 = cv::Vec3i(0,0,0);

        //transition variables
        TransitionObject t;

        //animation variables
        AnimationObject a;

        //testing purposes
        cv::Mat ledImage;

public:


    void update() override {
        updateTiming();
        if(settings.power){
            //-1 is the tv mode
            if(settings.mode == -1){
                using_webcam = true;
                //we need to get a specific pixel here
                a.setAnimImage(captureDevice.getImage());
                a.setAnimIndx(0);
                updateFromImageAnimation();
            }else {
                updateFromImageAnimation();
            }
            nac->render();
        } else if(t.getPercT() < 2.0f){
            //this block of code is in case we turn the power off on the leds we want to transition to power off and make sure they are turned off all the way
            if(t.getPercT() >= 1) t.setPercT(1.0f);  //set the transition percentage to 1 so we don't interpolate between some other value
            updateRGB(0, 0, 0);
            if(t.getPercT() >= 1) t.setPercT(3.0f); //if we did reach full power down, make sure to set the percentage to 3 so we don't keep updating the leds in the future
             nac->render();
        }
        //debug options
        if(show_LEDS){
            cv::imshow(name, ledImage);
            if( cv::waitKey (30) >= 0) {
                std::cout << "Closing" << std::endl;
                return;
            };
        }
    }
    
    void setPreset(std::string presetName) override {
        preset = presetName;
    }

    int usingTV() override {
        return settings.mode == -1;
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
        rgb2 = rgb;
        rgb = cv::Vec3i(0,0,0);
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

    void setName(json data){
        //if there is a name attribute in the json then update the name attribute, if not then keep it as the name it was prior unless empty then set it to default
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
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
        //set up the pinout
        if(!data["pin_out"].is_null()) nac->setPins(data["pin_out"]);
    }

    json getJson() override {
        // implementation for getting json on a LedDevice
        json data = {
                {"name", name}, // Add a new key-value pair to the JSON object
                {"type", "non-addressable"}, // Add another key-value pair named "response"
                {"settings", settings.getJson()},
                {"preset", preset},
                {"transition_speed", t.getTransitionSpeed()},
                // //depending on the type of the device the pinout will be different so change the response based on that.
                {"pin_out", nac->getPinsJson()},
            };
        return data;
    }

    NonAddressableLedDevice(std::string newName, int newType) {
        name = newName;
        t = TransitionObject();
        type = 0;
        if(type == 0){
            nac = new NonAddressableController();
        }
    }
    NonAddressableLedDevice(json data) {
        try{
            setName(data);
            type = 0;
            if(type == 0){
                nac = new NonAddressableController();
            }
            setData(data);
            std::cout << "Device: " << name << " type: " << "non-addressable" << std::endl;

            
            //initialize the transition object
            t = TransitionObject();
            a = AnimationObject();
            if(show_LEDS) ledImage = cv::Mat::zeros(cv::Size(1,1), CV_8UC3); // Update to use 3 color channels (CV_8UC3) instead of grayscale (0)
        }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            name = "default";
        }
    }

    void updateRGB(int r, int g, int b){
        if(t.getPercT() < 1){
            cv::Vec3i nColor = interpolate(rgb2, cv::Vec3i(r,g,b), t.getPercT());
            rgb = nColor;
            
        } else rgb = cv::Vec3i(r,g,b);
        nac->setColor(rgb[0], rgb[1], rgb[2]);
        //debug option to show the leds in an image
        if(show_LEDS) updateDebugPixel(rgb);
    }

    cv::Vec3i interpolate(cv::Vec3i color1, cv::Vec3i color2, float fraction){
        return cv::Vec3i((int) ((color2[0] - color1[0]) * fraction + color1[0]),
                        (int) ((color2[1] - color1[1]) * fraction + color1[1]),
                        (int) ((color2[2] - color1[2]) * fraction + color1[2]));
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
        cv::Vec3b pixel = cv::Vec3b(0,0,0);

        //get the average of the colors in the mapping
        for(int i = 0; i < settings.mappings.size(); i ++){
            int mapS = settings.mappings[i].mapSIndx;
            int mapE = settings.mappings[i].mapEIndx;
            if(mapS >= a.getCCols()) mapS = a.getCCols() - 1;
            if(mapE >= a.getCCols()) mapE = a.getCCols() - 1;
            if(mapS < 0) mapS = 0;
            if(mapE < 0) mapE = 0;
            int start = min(mapS, mapE);
            int end = max(mapS, mapE);
            for(int j = start; j <= end; j ++){
                if(j == start && i == 0){
                    pixel = row[j];
                } else {
                    cv::Vec3b nPixel = row[j];
                    pixel[0] = (pixel[0] + nPixel[0]) / 2;
                    pixel[1] = (pixel[1] + nPixel[1]) / 2;
                    pixel[2] = (pixel[2] + nPixel[2]) / 2;
                }
            }
        }
        updateRGB(pixel[2], pixel[1], pixel[0]);
    }

    //------- DEBUG ------
    void updateDebugPixel(cv::Vec3i nRGB){
        cv::Vec3b& pixel = ledImage.at<cv::Vec3b>(0,0);
        pixel[0] = nRGB[2]; // Blue channel
        pixel[1] = nRGB[1]; // Green channel
        pixel[2] = nRGB[0]; // Red channel
    }
};
