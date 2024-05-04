void InitSaveTVSettings() {
     svr.Post("/tv", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json requestJson = json::parse(req.body);
            captureDevice.setData(requestJson);
            saveTVSettings();
            json response = CreateResponse(true, "Updated TV Settings" , "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}