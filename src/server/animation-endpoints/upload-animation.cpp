void InitUploadAnimation() {
    svr.Post("/animations/upload", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000"); // Add this
        auto size = req.files.size();
        auto ret = req.has_file("file");
        std::cout<< "found a file" << std::endl;
        if (ret) {
            const auto& file = req.get_file_value("file");
            std::string path = "./resources/animations/" + file.filename;
            std::ofstream ofs(path, std::ios::binary);
            ofs << file.content;
            ofs.close(); // Close the file after writing
        }
        res.set_content(std::to_string(size), "text/plain");
    });
}