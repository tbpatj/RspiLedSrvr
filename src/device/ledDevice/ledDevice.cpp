#include "./pins.cpp"
#include "./settings.cpp"

class LedDevice : public Device {
private:
    std::string name;
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
    // void setName(const std::string& newName) override {
    //     name = newName;
    // }

    void update() override {
        // implementation for updating a LedDevice
        std::cout << name << std::endl;
    }

    void setData(json data) override {
        //if there is a name attribute in the json then update the name attribute, if not then keep it as the name it was prior unless empty then set it to default
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
        //set up the type of the device, if it's null then set it to non-addressable
        type = data["type"].is_null() ? type : (data["type"] == "addressable" ? 1 : 0);
        //set up the settings object
        if (!data["settings"].is_null()) {
            settings.setData(data["settings"]);
        }
    }

    void getJson() override {
        // implementation for getting json on a LedDevice
    }

    LedDevice(std::string newName, int newType) {
        name = newName;
        type = newType;
        // settings = new LedDeviceSettings("default", 0, "default", false);
        pins = LedDevicePins(0);
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