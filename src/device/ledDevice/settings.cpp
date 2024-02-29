class LedDeviceSettings {
    public:
        std::string name;
        int mode;
        bool power;
        int type;

    int getModeFromStr(std::string modeStr){
        if(modeStr == "default"){
            return 0;
        } else if(modeStr == "rainbow"){
            return 1;
        } else if(modeStr == "color"){
            return 2;
        } else if(modeStr == "off"){
            return 3;
        } else {
            return 0;
        }
    }

    std::string getStrFromMode(){
        if(mode == 0){
            return "default";
        } else if(mode == 1){
            return "rainbow";
        } else if(mode == 2){
            return "color";
        } else if(mode == 3){
            return "off";
        } else {
            return "default";
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