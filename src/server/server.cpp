#include "./responses/server-responses.cpp"
#include "./serve-client.cpp"
#include "./device-endpoints/devices.cpp"

void RunLedServer() {
    ServeClient();
    InitDeviceEndpoints();
    std::cout << "Starting server" << std::endl;
    svr.listen("0.0.0.0", 3000);
}