#include "./responses/server-responses.cpp"
#include "./serve-client.cpp"
#include "./device-endpoints/devices.cpp"
#include "./animation-endpoints/init-animations.cpp"
#include "./preset-endpoints/init-presets.cpp"

void RunLedServer() {
    ServeClient();
    InitDeviceEndpoints();
    InitPresetEndpoints();
    InitAnimationEndpoints();
    std::cout << "Starting server" << std::endl;

    // Custom middleware to handle OPTIONS request and set CORS headers
    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Max-Age", "86400");
    });

    svr.listen("0.0.0.0", 3001);
}