void InitDeleteDevice() {
    svr.Delete(R"(/devices/delete/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        //get the name param from the endpoint
        std::smatch matches;
        std::regex_match(req.path, matches, std::regex(R"(/updateDevice/(\w+))"));
        //if no name param was supplied then return back
        if(matches.size() <= 1){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "not enough info was passed in the endpoint. Use as /updateDevice/:name", "404");
            res.set_content(response.dump(), "application/json");
            return;
        }
        //get the name parameter
        std::string name = matches[1].str();
        json requestJson = json::parse(req.body);
        std::string type = requestJson["type"].is_null() ? "none" : requestJson["type"];
        if(type == "none"){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "not enough info was passed in the endpoint. Please make sure to include the type of the device in the body", "404");
            res.set_content(response.dump(), "application/json");
            return;
        }
        //Find if the device with the name and type exists
        int index = -1;
        for (int i = 0; i < devices.size(); i++) {
            if (devices[i]->getName() == name && devices[i]->getStrType() == type){
                index = i;
                break;
            }
        }
        //if the device wasn't found return that we couldn't find the device
        if(index == -1){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "Device not found", "404", requestJson);
            res.set_content(response.dump(), "application/json");
            return;
        }
        //update the found device or create the new device
         try {
            json requestJson = json::parse(req.body);
            try{
               //delete the device from our devices list
                for (int i = presets.size() - 1; i >= 0; i--) {
                    if (presets[i]->getName() == name && presets[i]->getStrType() == type){
                        presets.erase(presets.begin() + i);
                    }
                }
                devices.erase(devices.begin() + index);
                
            }catch(const json::exception& e){
                //catch any json parsing errors then send that back
                std::cerr << "Error deleting device: " << e.what() << std::endl;
                res.status = 400;  // Bad Request
                json response = CreateResponse(false, "trouble deleting the device", "500", requestJson);
                res.set_content(response.dump(), "application/json");
                return;
            }
            saveDevices();
            json response = CreateResponse(true, "Device deleted successfully", "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            json response = CreateResponse(false, "trouble parsing json 2", "500", requestJson);
            res.set_content(response.dump(), "application/json");
         }
    });
}