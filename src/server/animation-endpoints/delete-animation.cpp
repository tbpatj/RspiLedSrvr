void InitDeleteAnimation() {
    svr.Delete(R"(/animations/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        //get the name param from the endpoint
        std::smatch matches;
        std::regex_match(req.path, matches, std::regex(R"(/animations/(\w+))"));
        //if no name param was supplied then return back
        if(matches.size() <= 1){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "not enough info was passed in the endpoint. Use as /delete/:name", "404");
            res.set_content(response.dump(), "application/json");
            return;
        }
        //get the name parameter
        std::string name = matches[1].str();
        //see if we can find the animation in our library
        int found = -1;
        for(int i = 0; i < animations.size(); i++){
            if(animations[i].getName() == name){
                found = i;
                break;
            }
        }
        //if the animation wasn't found return that we couldn't find the animation
        if(found == -1){
            res.status = 404;  // Not Found
            json response = CreateResponse(false, "Animation not found", "404");
            res.set_content(response.dump(), "application/json");
            return;
        }
        //attempt to delete the actual file
        bool deleted = false;
        try {
            std::experimental::filesystem::remove("./resources/animations/" + animations[found].getName() + animations[found].getExtension());
            std::cout << "File deleted successfully." << std::endl;
            deleted = true;
        } catch (const std::experimental::filesystem::filesystem_error& e) {
            std::cerr << "Error deleting file: " << e.what() << std::endl;
        }
        //if it failed to delete the file return back to the user.
        if(deleted == false){
            res.status = 500;  // Internal Server Error
            json response = CreateResponse(false, "trouble deleting the animation", "500");
            res.set_content(response.dump(), "application/json");
            return;
        }
        //if it did delete the file then remove it from our list and then return back to the user
        animations.erase(animations.begin() + found);
        json response = CreateResponse(true, "Animation Deleted", "200");
        res.set_content(response.dump(), "application/json");
    });
}