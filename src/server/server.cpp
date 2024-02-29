#include "./responses/server-responses.cpp"
#include "./serve-client.cpp"
#include "./device-endpoints/devices.cpp"

void RunLedServer() {
    ServeClient();
    InitDeviceEndpoints();

    svr.listen("0.0.0.0", 3000);
}