void InitAddDevice() {
     svr.Post("/addDevice", [](const httplib::Request& req, httplib::Response& res) {
         try {
            json requestJson = json::parse(req.body);

            
            json response = CreateResponse(true, "Device added successfully", "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}