void InitGetDebugImage() {
     svr.Get("/debug-image", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            captureDevice.startDebugImage();
            if(devices.size() > 0){
                devices[0]->startDebugOutput();
            }
            std::this_thread::sleep_for(std::chrono::seconds(5));
            cv::Mat image = captureDevice.stopDebugImage();
            cv::Mat image2;
            if(devices.size() > 0)
                image2 = devices[0]->stopDebugOutput();


            // Create JSON object with image data
            json data;
            data["image"] = convertMatToBase64(image);;
            data["led_image"] = convertMatToBase64(image2);
            data["diff_image"] = convertMatToBase64(getDifferenceImage(image));
            data["diff_led_image"] = convertMatToBase64(getDifferenceImage(image2));;

            // Create response JSON
            json response = CreateResponse(true, "Debug info retrieved", "200", data);
            res.set_content(response.dump(), "application/json"); // Convert response to string and send as JSON
         }catch(const json::exception& e){
             std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            res.status = 400;  // Bad Request
            res.set_content("Error parsing JSON", "text/plain");
         }
    });
}