#include "./file-manage.cpp"

FileManage devicesManager = FileManage("devices.json");

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