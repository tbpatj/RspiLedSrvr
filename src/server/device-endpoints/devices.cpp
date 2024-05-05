#include "./add-device.cpp"
#include "./update-device.cpp"
#include "./get-devices.cpp"
#include "./delete-device.cpp"
#include "./show-mapping.cpp"

void InitDeviceEndpoints() {
    InitGetDevices();
    InitAddDevice();
    InitUpdateDevice();
    InitDeleteDevice();
    InitShowMapping();
}