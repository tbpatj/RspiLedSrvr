void InitShowMapping() {
     svr.Post(R"(/show-mapping/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         //check to see if the user passed in a index
        //get the name param from the endpoint
        std::smatch matches;
        std::regex_match(req.path, matches, std::regex(R"(/show-mapping/(\w+))"));
        //if no name param was supplied then return back
        if(matches.size() <= 1){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "not enough info was passed in the endpoint. Use as /show-mapping/:name, then pass in the required body {index: *num*}", "404");
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
        

         try {
            //check if an index was passed in the body of the request
            json requestJson = json::parse(req.body);
            if(requestJson["index"].is_null()){
                res.status = 400;  // Bad Request
                json response = CreateResponse(false, "Index not found in body", "400", requestJson);
                res.set_content(response.dump(), "application/json");
                return;
            }
            int mapIndex = requestJson["index"];
            devices[index]->showMapping(mapIndex);

            json response = CreateResponse(true, "Devices found and retrieved", "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}