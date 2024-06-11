#include "./responses/server-responses.cpp"
#include "./serve-client.cpp"
#include "./device-endpoints/devices.cpp"
#include "./animation-endpoints/init-animations.cpp"
#include "./preset-endpoints/init-presets.cpp"
#include "./capture-device-endpoints/init.cpp"
#include "./util-endpoints/init-utils.cpp"
#include "./tv-endpoints/tv.cpp"

void RunLedServer() {

    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000, http://localhost:3001");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS, DELETE");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Max-Age", "86400");
    });

    InitDeviceEndpoints();
    InitPresetEndpoints();
    InitAnimationEndpoints();
    InitCaptureDeviceEndpoints();
    InitTVEndpoints();
    InitUtilsEndpoints();
    ServeClient();

    std::cout << "Starting server" << std::endl;

    // Custom middleware to handle OPTIONS request and set CORS headers

    svr.listen("0.0.0.0", 3001);
}