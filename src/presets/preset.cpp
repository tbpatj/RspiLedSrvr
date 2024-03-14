class Preset{
    private:
        std::string name = "default";
        std::string deviceName = "";
        std::string deviceType = "addressable";
        json data;

    public:
        void setName(std::string nName) {
            name = nName;
        };
        std::string getName() {
            return name;
        };
        std::string getDeviceName() {
            return deviceName;
        };
        void setDeviceName(std::string nDeviceName) {
            deviceName = nDeviceName;
        }; 
        std::string getDeviceType(){
            return deviceType;
        };
        void setDeviceType(std::string nDeviceType) {
            deviceType = nDeviceType;
        };
        void setJson(json nData) {
            data = nData;
        };
        json getJson() {
            return data;
        };

    Preset(std::string nName, std::string deviceName, std::string deviceType, json nData){
        name = nName;
        deviceName = deviceName;
        deviceType = deviceType;
        data = nData;
    }
    Preset(json nData){
        name = nData["name"].is_null() ? name : static_cast<std::string>(nData["name"]);
        deviceName = nData["device_name"].is_null() ? deviceName : static_cast<std::string>(nData["device_name"]);
        deviceType = nData["device_type"].is_null() ? deviceType : static_cast<std::string>(nData["device_type"]);
        data = nData;
    }
};