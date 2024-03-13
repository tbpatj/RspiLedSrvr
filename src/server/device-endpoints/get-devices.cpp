void InitGetDevices() {
     svr.Get("/devices", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json devicesJson = json::array();
            for (int i = 0; i < devices.size(); i++) {
                devicesJson.push_back(devices[i]->getJson());
            }
            json response = CreateResponse(true, "Devices found and retrieved", "200", devicesJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}