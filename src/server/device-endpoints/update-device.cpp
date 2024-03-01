void InitUpdateDevice() {
    svr.Post(R"(/updateDevice/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
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
        //Find if the device with the name exists
        int index = -1;
        for (int i = 0; i < devices.size(); i++) {
            if (devices[i]->getName() == name) {
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
            std::cout << req.body << std::endl;
            json requestJson = json::parse(req.body);
            try{
                devices[index]->setData(requestJson);
            }catch(const json::exception& e){
                //catch any json parsing errors then send that back
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                res.status = 400;  // Bad Request
                json response = CreateResponse(false, "trouble parsing json", "400", requestJson);
                res.set_content(response.dump(), "application/json");
                return;
            }
            saveDevices();
            json response = CreateResponse(true, "Device updated successfully", "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            json response = CreateResponse(false, "trouble parsing json 2", "400", requestJson);
            res.set_content(response.dump(), "application/json");
         }
    });
}