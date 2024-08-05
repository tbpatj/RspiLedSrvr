void InitGetDebugImage() {
     svr.Get("/debug-image", [](const httplib::Request& req, httplib::Response& res) {
         res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
         try {
            captureDevice.startDebugImage();
            std::this_thread::sleep_for(std::chrono::seconds(4));
            cv::Mat image = captureDevice.stopDebugImage();

            // Convert image to base64-encoded string
            std::vector<uchar> buffer;
            cv::imencode(".jpg", image, buffer);
            std::string imageBase64 = base64_encode(buffer.data(), buffer.size());

            // Create JSON object with image data
            json data;
            data["image"] = imageBase64;

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