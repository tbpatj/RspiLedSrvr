class Preset{
    private:
        std::string name = "default";
        std::string deviceName = "";
        std::string deviceType = "non-addressable";
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

        int getNumType() {
            return deviceType == "addressable" ? 1 : 0;
        }

    Preset(std::string nName, std::string deviceName, std::string deviceType, json nData){
        name = nName;
        deviceName = deviceName;
        deviceType = deviceType;
        data = nData;
    }
    Preset(json nData){
        std::cout << "Preset: " << nData["name"] << " device_name: " << nData["device_name"] << " device_type: " << nData["device_type"] << std::endl;
        name = nData["name"].is_null() ? name : static_cast<std::string>(nData["name"]);
        deviceName = nData["device_name"].is_null() ? deviceName : static_cast<std::string>(nData["device_name"]);
        deviceType = nData["device_type"].is_null() ? deviceType : static_cast<std::string>(nData["device_type"]);
        data = nData;
    }
};