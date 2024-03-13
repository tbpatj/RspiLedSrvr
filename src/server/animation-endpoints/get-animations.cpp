void InitGetAnimations() {
     svr.Get("/modes", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json animationsJson = json::array();
            for (int i = 0; i < animations.size(); i++) {
                animationsJson.push_back(animations[i].getName());
            }
            //include the other modes that we have
            animationsJson.push_back("tv");
            json response = CreateResponse(true, "Animation/modes found and retrieved", "200", animationsJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}