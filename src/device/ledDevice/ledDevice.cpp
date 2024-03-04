#include "./pins.cpp"
#include "./settings.cpp"

class LedDevice : public Device {
private:
        //types are
        //0 for non-addressable LED
        //1 for addressable LED
        int type;
        LedDeviceSettings settings = LedDeviceSettings("default", 0, "default", false);
        LedDevicePins pins;
        int ledCount;

        //TV stuff
        int led_pos[4][2] = {{0,0},{0,0},{0,0},{0,0}};
        std::vector<int> leds;
        std::vector<int> leds2;

        //transition stuff
        int t;
        int transitionSpeed;

public:

    void update() override {
        // implementation for updating a LedDevice
        if(settings.power){
            if(type == 0) {
                //update non-addressable leds
            } else if(type == 1) {
                //update addressable leds

                //tv update mode
                if(settings.mode == 1){
                    updateLedTVStyle();
                }
            }
        }
    }

    void setData(json data) override {
        //if there is a name attribute in the json then update the name attribute, if not then keep it as the name it was prior unless empty then set it to default
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
        //set up the type of the device, if it's null then set it to non-addressable
        type = data["type"].is_null() ? type : (data["type"] == "addressable" ? 1 : 0);
        //led count
        ledCount = data["led_count"].is_null() ? ledCount : static_cast<int>(data["led_count"]);
        //set up the settings object
        if (!data["settings"].is_null()) {
            settings.setData(data["settings"]);
        }
        if(type == 0) pins.setData(data["pin_out"]);
        if(type == 1) pins.setData(static_cast<int>(data["pin_out"]));
        if(!data["tv_settings"].is_null()){
            led_pos[0][0] = data["tv_settings"]["tops"];
            led_pos[0][1] = data["tv_settings"]["tope"];
            led_pos[1][0] = data["tv_settings"]["bottoms"];
            led_pos[1][1] = data["tv_settings"]["bottome"];
            led_pos[2][0] = data["tv_settings"]["lefts"];
            led_pos[2][1] = data["tv_settings"]["lefte"];
            led_pos[3][0] = data["tv_settings"]["rights"];
            led_pos[3][1] = data["tv_settings"]["righte"];
        }
    }

    json getTVSettings() {
        json data = {
            {"tops", led_pos[0][0]},
            {"tope", led_pos[0][1]},
            {"bottoms", led_pos[1][0]},
            {"bottome", led_pos[1][1]},
            {"lefts", led_pos[2][0]},
            {"lefte", led_pos[2][1]},
            {"rights", led_pos[3][0]},
            {"righte", led_pos[3][1]}
        };
        return data;
    }

    json getJson() override {
        // implementation for getting json on a LedDevice
        json data = {
                {"name", name}, // Add a new key-value pair to the JSON object
                {"type", type == 1 ? "addressable" : "non-addressable"}, // Add another key-value pair named "response"
                {"settings", settings.getJson()},
                // //depending on the type of the device the pinout will be different so change the response based on that.
                {"pin_out", type == 1 ? static_cast<json>(pins.getAddressablePinout()) : static_cast<json>(pins.getNonAddressableJson())},
                {"tv_settings", getTVSettings()}
            };
        return data;
    }

    LedDevice(std::string newName, int newType) {
        name = newName;
        type = newType;
        // settings = new LedDeviceSettings("default", 0, "default", false);
        pins = LedDevicePins(0);
    }
    LedDevice(json data) {
        try{
            setData(data);
        }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            name = "default";
            type = 0;
        }
    }

    void updateLedTVStyle(){
        int start = 0;
        int end = 0;
        int length = 0;
        int increment = 1;
        int startI = 0;
        int iterations = 0;
        float step = 1.0f;
        if(captureDevice.isCapturing){
            for (int i = 0; i < 4; i++) {
                start = led_pos[i][0];
                end = led_pos[i][1];
                iterations = captureDevice.getIterations(i);

                cv::Vec3b* row = captureDevice.getPrcsdRwFrmSide(i);
                //set up the loop values so we go in the correct direction
                length = end - start;
                if(length < 0){
                    increment = -1;
                    startI = length;
                } else {
                    increment = 1;
                    startI = 0;
                }
                step = length / iterations;
                for(int j = 0; j >= 0 && j < length; j = j + increment) {
                    //update the led color with the closest color in the image frame
                    //updateLED(i, captureDevice.getClosestColor(x, y));
                }
                //update the led color with the closest color in the image frame
                //updateLED(i, captureDevice.getClosestColor(x, y));
            }
        } else {
            //tv isn't capturing. Maybe do some idle animation here
        }
    }
};

// class LedDevice : public Device {
    // private:
    //     std::string name;
    //     int type;
    
    // public:
    //     void setName(std::string iName) {
    //         name = iName;
    //     }
    //     std::string getName() {
    //         return name;
    //     }

    //     void setType(int iType) {
    //         type = iType;
    //     }
    //     int getType() {
    //         return type;
    //     }

    // Device(std::string iName, int iType) {
    //     name = iName;
    //     type = iType;
    // }



    //Class I was creating earlier
    // private:
    //     std::string name;
    //     //types are
    //     //0 for non-addressable LED
    //     //1 for addressable LED
    //     int type;
    //     DeviceSettings settings;
    //     DevicePins pins;
    //     int ledCount;

    //     //TV stuff
    //     int led_pos[4][2] = {{0,0},{0,0},{0,0},{0,0}};
    //     std::vector<int> leds;
    //     std::vector<int> leds2;

    //     //transition stuff
    //     int t;
    //     int transitionSpeed;


    // /* --------- LED UTIL CODE --------- */

    // void setLEDCount(int count) {
    //     ledCount = count;
    //     leds.resize(ledCount);
    //     leds2.resize(ledCount);
    // }

    // void setLED(int index, int r, int g, int b) {
    //     leds[index] = (r << 16) | (g << 8) | (b);
    // }

    // /**
    // * interpolate 2 RGB colors
    // * @param color1    integer containing color as 0x00RRGGBB
    // * @param color2    integer containing color as 0x00RRGGBB
    // * @param fraction  how much interpolation (0..1)
    // * - 0: full color 1
    // * - 1: full color 2
    // * @return the new color after interpolation
    // */
    // int interpolate(int color1, int color2, float fraction)
    // {
    //         //bit shift the colors back
    //         unsigned char   r1 = (color1 >> 16) & 0xff;
    //         unsigned char   r2 = (color2 >> 16) & 0xff;
    //         unsigned char   g1 = (color1 >> 8) & 0xff;
    //         unsigned char   g2 = (color2 >> 8) & 0xff;
    //         unsigned char   b1 = color1 & 0xff;
    //         unsigned char   b2 = color2 & 0xff;

    //         //interpolate between the colors
    //         return (int) ((r2 - r1) * fraction + r1) << 16 |
    //                 (int) ((g2 - g1) * fraction + g1) << 8 |
    //                 (int) ((b2 - b1) * fraction + b1);
    // }

    // void updateLED(int index) {
    //     if(type == 0) {
    //         //update the led color in the device with color
    //     } else {
    //         //update the led color with the closest color in the image frame
    //     }
    // }

    // /** --------- LED LOOP --------- */
    // void renderLEDS() {
    //     for (int i = 0; i < ledCount; i++) {
    //         //if we're not transitioning then we don't need to perform interpolation
    //         if(transitionSpeed != 0 && t < transitionSpeed){
    //             int color = interpolate(leds2[i], leds[i], t / transitionSpeed);
    //             //update the led color in the device with color

    //         } else {
    //             //just update the led color with the led vector
    //         }
    //     }
    // }

    // Device() {
    //     name = "default";
    //     type = 0;
    //     settings = LedDeviceSettings("default", 0, "default", false);
    //     pins = LedDevicePins(0, 0, 0);
    //     ledCount = 0;

    //     //transition stuff declaration
    //     t=0;
    //     transitionSpeed = 0;
    // }
// };