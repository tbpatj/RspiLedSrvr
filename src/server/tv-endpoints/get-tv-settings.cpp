void InitGetTVSettings() {
     svr.Get("/tv", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json tvSettings = captureDevice.getJson();
            json response = CreateResponse(true, "TV Settings Retrieved", "200", tvSettings);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}