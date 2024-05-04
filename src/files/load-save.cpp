#include "./file-manage.cpp"

FileManage devicesManager = FileManage("resources/devices.json");
FileManage presetsManager = FileManage("resources/presets.json");
FileManage tvSettingsManager = FileManage("resources/tv_settings.json");

void loadDevices() {
    std::cout << "-------- Loading Devices ---------" << std::endl;
    std::string content = devicesManager.read();
    if(content != ""){
        json devicesJson = json::parse(content);
        for (json::iterator it = devicesJson.begin(); it != devicesJson.end(); ++it) {
            std::string type = it.value()["type"];
            if(type == "addressable"){
                std::unique_ptr<Device> ledDevice = std::make_unique<AddressableLedDevice>(it.value());
                devices.push_back(std::move(ledDevice));
            } else if(type == "non-addressable"){
                std::unique_ptr<Device> ledDevice = std::make_unique<NonAddressableLedDevice>(it.value());
                devices.push_back(std::move(ledDevice));
            }
        }
    }
    std::cout << "----------------------------------\n" << std::endl;
}


void saveDevices() {
    json devicesJson = json::array();
    for (int i = 0; i < devices.size(); i++) {
        devicesJson.push_back(devices[i]->getJson());
    }
    devicesManager.write(devicesJson.dump());
}


void loadPresets() {
    std::cout << "-------- Loading Presets ---------" << std::endl;
    std::string content = presetsManager.read();
    if(content != ""){
        json presetsJson = json::parse(content);
        for (json::iterator it = presetsJson.begin(); it != presetsJson.end(); ++it) {
            json presetJson = it.value();
            presets.push_back(Preset(presetJson));
        }
    }
    std::cout << "----------------------------------\n" << std::endl;
}


void savePresets() {
    json presetsJson = json::array();
    for (int i = 0; i < presets.size(); i++) {
        presetsJson.push_back(presets[i].getJson());
    }
    presetsManager.write(presetsJson.dump());
}

void loadTVSettings() {
    std::cout << "-------- Loading TV Settings ---------" << std::endl;
    std::string content = tvSettingsManager.read();
    if(content != ""){
        json tvSettingsJson = json::parse(content);
        captureDevice.setData(tvSettingsJson);
    }
    // std::cout << "----------------------------------\n" << std::endl;
}

void saveTVSettings() {
    json tvSettings = captureDevice.getJson();
    tvSettingsManager.write(tvSettings.dump());
}

void loadDeviceAndPresets(){
    loadDevices();
    loadPresets();
    loadTVSettings();
}