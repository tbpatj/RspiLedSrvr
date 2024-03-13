#include "./ledMapping.cpp"

class LedDeviceSettings {
    public:
        std::string name;
        int mode;
        bool power;
        int type;
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
        } else if(mode > 0 && mode < animations.size()){
            return animations[mode].getName();
        } else {
            return "none";
        }
    }

    LedDeviceSettings(){
        name = "default";
        type = 0;
        mode = 0;
        power = false;
    }

    void setData(json data) {
        name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
        type = data["type"].is_null() ?  type : data["type"] == "addressable" ? 1 : 0;
        mode = data["mode"].is_null() ?  mode : getModeFromStr(static_cast<std::string>(data["mode"]));
        power = data["power"].is_null() ? power : data["power"] == "on" ? true : false;
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
                {"type", type == 1 ? "addressable" : "non-addressable"}, // Add another key-value pair named "response"
                {"mode", getStrFromMode()},
                {"power", power ? "on" : "off"},
                {"mapping", getMappings()}
            };
        return data;
    }

    LedDeviceSettings(std::string inName, int inType,int inMode, bool inPower) {
        name = inName;
        type = inType;
        mode = inMode;
        power = inPower;
    }
    LedDeviceSettings(std::string inName, int inType,std::string inMode, bool inPower) {
        name = inName;
        type = inType;
        mode = getModeFromStr(inMode);
        power = inPower;
    }
};