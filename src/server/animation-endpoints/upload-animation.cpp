void InitUploadAnimation() {
    svr.Post("/animations/upload", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000"); // Add this
        auto size = req.files.size();
        auto ret = req.has_file("file");
        //if a file was actually passed then we will process it
        if (ret) {
            const auto& file = req.get_file_value("file");
            //get filename and extension
            std::string filename = file.filename.substr(0, file.filename.find_last_of("."));
            std::string extension = file.filename.substr(file.filename.find_last_of("."));
            //check if the file already exists
            for(int i = 0; i < animations.size(); i++){
                if(animations[i].getName() == filename){
                    json response = CreateResponse(false, "Animation already exists", "400");
                    res.status = 400;
                    res.set_content(response.dump(), "application/json");
                    return;
                }
            }

            //save the file to the folder
            std::string path = "./resources/animations/" + file.filename;
            std::ofstream ofs(path, std::ios::binary);
            ofs << file.content;
            ofs.close(); // Close the file after writing
            

            // add the animation to the animations vector
            Animation animation(filename, extension);
            animations.push_back(animation);
            
        }
        json response = CreateResponse(true, "Animation Added to Library", "200");
        res.set_content(response.dump(), "application/json");
    });
}