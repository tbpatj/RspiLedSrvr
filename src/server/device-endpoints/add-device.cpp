void InitAddDevice() {
     svr.Post("/addDevice", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json requestJson = json::parse(req.body);
            try{
                // Check if a device with the same name already exists
                std::string requestedDeviceName = requestJson["name"];
                bool deviceExists = false;
                for (const auto& device : devices) {
                    if (device->getName() == requestedDeviceName) {
                        deviceExists = true;
                        break;
                    }
                }
                if (deviceExists) {
                    // Device with the same name already exists
                    json response = CreateResponse(false, "Device with the same name already exists", "400", requestJson);
                    res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
                    return;
                }
                
                if(requestJson["type"] == "addressable"){
                    devices.push_back(std::make_unique<AddressableLedDevice>(requestJson));
                    presets.push_back(Preset(requestJson["settings"]));
                    presets.push_back(Preset(getSleepPreset(requestJson["settings"])));
                } else if(requestJson["type"] == "non-addressable"){
                    devices.push_back(std::make_unique<NonAddressableLedDevice>(requestJson));
                    presets.push_back(Preset(requestJson["settings"]));
                    presets.push_back(Preset(getSleepPreset(requestJson["settings"])));
                }
            }catch(const json::exception& e){
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                res.status = 400;  // Bad Request
                res.set_content("Error parsing JSON", "text/plain");
                return;
            }
            //save the actual files
            saveDevices();
            savePresets();
            json response = CreateResponse(true, "Device added successfully", "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}