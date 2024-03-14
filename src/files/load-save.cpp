#include "./file-manage.cpp"

FileManage devicesManager = FileManage("devices.json");
FileManage presetsManager = FileManage("presets.json");

void loadDevices() {
    std::string content = devicesManager.read();
    if(content != ""){
        json devicesJson = json::parse(content);
        for (json::iterator it = devicesJson.begin(); it != devicesJson.end(); ++it) {
            std::string type = it.value()["type"];
            if(type == "addressable" || type == "non-addressable"){
                std::unique_ptr<Device> ledDevice = std::make_unique<LedDevice>(it.value());
                devices.push_back(std::move(ledDevice));
            }
        }
    }
}


void saveDevices() {
    json devicesJson = json::array();
    for (int i = 0; i < devices.size(); i++) {
        devicesJson.push_back(devices[i]->getJson());
    }
    devicesManager.write(devicesJson.dump());
}


void loadPresets() {
    std::string content = presetsManager.read();
    if(content != ""){
        json presetsJson = json::parse(content);
        for (json::iterator it = presetsJson.begin(); it != presetsJson.end(); ++it) {
            std::string name = it.value()["name"];
            std::string deviceName = it.value()["device_name"];
            std::string deviceType = it.value()["device_type"];
            json presetJson = it.value();
            presets.push_back(Preset(name, deviceName, deviceType, presetJson));
        }
    }
}

void savePresets() {
    json presetsJson = json::array();
    for (int i = 0; i < presets.size(); i++) {
        presetsJson.push_back(presets[i].getJson());
    }
    presetsManager.write(presetsJson.dump());
}

void loadDeviceAndPresets(){
    loadDevices();
    loadPresets();
}