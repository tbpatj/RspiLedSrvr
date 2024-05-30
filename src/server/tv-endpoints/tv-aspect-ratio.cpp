void InitTVAspectRatio() {
     svr.Post("/tv/aspect-ratio", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            json requestJson = json::parse(req.body);
            
            if(requestJson["aspect_ratio"].is_null()){
                res.status = 400;  // Bad Request
                res.set_content("aspect_ratio is required", "text/plain");
                return;
            } else if(requestJson["aspect_ratio"] == "21:9"){
                captureDevice.setAspectRatio(21, 9);
            } else if(requestJson["aspect_ratio"] == "16:9"){
                captureDevice.setAspectRatio(16, 9);
            } else if(requestJson["aspect_ratio"] == "4:3"){
                captureDevice.setAspectRatio(4, 3);
            } else if(requestJson["aspect_ratio"] == "1:1"){
                captureDevice.setAspectRatio(1, 1);
            } else {
                std::string ratio = requestJson["aspect_ratio"];
                int indx = 0;
                bool passed = false;
                std::string w = "";
                std::string h = "";
                while(indx < ratio.size()){
                    if(ratio[indx] == ':'){
                        passed = true;
                    } else if(passed){
                        h += ratio[indx];
                    } else {
                        w += ratio[indx];
                    }
                    indx++;
                }
                if(w.size() > 0 && h.size() > 0){
                    try{
                        int width = std::stoi(w);
                        int height = std::stoi(h);
                        captureDevice.setAspectRatio(width, height);
                    } catch(const std::invalid_argument& e){
                        res.status = 400;  // Bad Request
                        json response = CreateResponse(false, "Invalid aspect_ratio","400");
                        res.set_content(response.dump(), "application/json");
                        return;
                    }
                } else {
                    res.status = 400;  // Bad Request
                    json response = CreateResponse(false, "Invalid aspect_ratio","400");
                    res.set_content(response.dump(), "application/json");
                    return;
                }
            }

            json response = CreateResponse(true, "Updated TV Aspect Ratio" , "200", requestJson);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            json response = CreateResponse(false, "Error parsing json","400");
            res.set_content(response.dump(), "application/json");
         }
    });
}