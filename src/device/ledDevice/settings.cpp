#include "./ledMapping.cpp"

class LedDeviceSettings {
    public:
        std::string name;
        std::string deviceName;
        //currently means it's unassigned, since 0 could be a mode that exists
        int mode = -1000;
        bool power;
        int deviceType;
         //amount of time it takes to move to the next frame in miliseconds
        long long animSpeed = 1000;
        std::vector<LedDeviceMapping> mappings;

    int getModeFromStr(std::string modeStr){
        if(modeStr == "tv"){
            return -1;
        } else {
            //see if we can find the animation index if it matches the name of another animation in our library
            for (int i = 0; i < animations.size(); i++) {
                if (animations[i].getName() == modeStr) {
                    return i;
                }
            }
            return 0;
        }
    }

    std::string getStrFromMode(){
        if(mode == -1){
            return "tv";
        } else if(mode >= 0 && mode < animations.size()){
            return animations[mode].getName();
        } else {
            return "none";
        }
    }

    LedDeviceSettings(){
        name = "default";
        deviceType = 0;
        mode = -1000;
        power = false;
    }

    void setData(json data) {
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
        deviceType = data["device_type"].is_null() ?  deviceType : data["device_type"] == "addressable" ? 1 : 0;
        mode = data["mode"].is_null() ?  mode : getModeFromStr(static_cast<std::string>(data["mode"]));
        power = data["power"].is_null() ? power : data["power"] == "on" ? true : false;
        deviceName = data["device_name"].is_null() ? (deviceName) : static_cast<std::string>(data["device_name"]);
        //update timings
        animSpeed = data["animation_speed"].is_null() ? animSpeed : static_cast<long long>(data["animation_speed"]);
        setMappings(data);
    }

    void setMappings(json data) {
        if (data.contains("mapping") && data["mapping"].is_array()) {
            json mappingData = data["mapping"];
            mappings.clear();
            for (const auto& mapping : mappingData) {
                if (mapping.contains("ledSIndx") && mapping.contains("ledEIndx") && mapping.contains("mapSIndx") && mapping.contains("mapEIndx")) {
                    int ledSIndx = static_cast<int>(mapping["ledSIndx"]);
                    int ledEIndx = static_cast<int>(mapping["ledEIndx"]);
                    int mapSIndx = static_cast<int>(mapping["mapSIndx"]);
                    int mapEIndx = static_cast<int>(mapping["mapEIndx"]);
                    mappings.push_back({ledSIndx, ledEIndx, mapSIndx, mapEIndx});
                }
            }
        }
    }

    json getMappings() {
        json data = json::array();
        for (const auto& mapping : mappings) {
            json mappingData = {
                {"ledSIndx", mapping.ledSIndx},
                {"ledEIndx", mapping.ledEIndx},
                {"mapSIndx", mapping.mapSIndx},
                {"mapEIndx", mapping.mapEIndx}
            };
            data.push_back(mappingData);
        }
        return data;
    }

    json getJson() {
        json data = {
                {"name", name}, // Add a new key-value pair to the JSON object
                {"device_type", deviceType == 1 ? "addressable" : "non-addressable"}, // Add another key-value pair named "response"
                {"device_name", deviceName},
                {"mode", getStrFromMode()},
                {"power", power ? "on" : "off"},
                {"mapping", getMappings()},
                {"animation_speed", animSpeed},
            };
        return data;
    }

    LedDeviceSettings(std::string inName, int inDeviceType,int inMode, bool inPower) {
        name = inName;
        deviceType = inDeviceType;
        mode = inMode;
        power = inPower;
    }
    LedDeviceSettings(std::string inName, int inDeviceType,std::string inMode, bool inPower) {
        name = inName;
        deviceType = inDeviceType;
        mode = getModeFromStr(inMode);
        power = inPower;
    }
};