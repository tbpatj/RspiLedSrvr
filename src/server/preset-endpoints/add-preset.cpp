void InitAddPreset() {
     svr.Post("/presets", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json requestJson = json::parse(req.body);
            std::string successString = "Preset added successfully.";
            try{
                Preset preset(requestJson["settings"]);
                int foundIndx = -1;
                for(int i = 0; i < presets.size(); i ++){
                    if(presets[i].getName() == preset.getName() && presets[i].getDeviceName() == preset.getDeviceName() && presets[i].getDeviceType() == preset.getDeviceType()){
                        foundIndx = i;
                        break;
                    }
                }
                //the preset was not found so create a new preset
                if(foundIndx == -1) {
                    presets.push_back(preset);
                } else {
                    //the preset was found so update the current preset
                    presets[foundIndx] = preset;
                    successString = "Preset updated successfully.";
                }
                //update the presets
                savePresets();
                //update the device, so we know what preset we are using
                int index = -1;
                for (int i = 0; i < devices.size(); i++) {
                    if (devices[i]->getName() == preset.getDeviceName()) {
                        index = i;
                        break;
                    }
                }
                if(index != -1){
                    devices[index]->setPreset(preset.getName());
                    saveDevices();
                }
            
            }catch(const json::exception& e){
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                res.status = 400;  // Bad Request
                res.set_content("Error parsing JSON", "text/plain");
                return;
            }

            json response = CreateResponse(true, successString , "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}