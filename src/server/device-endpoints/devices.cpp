#include "./add-device.cpp"
#include "./update-device.cpp"
#include "./get-devices.cpp"

void InitDeviceEndpoints() {
    InitGetDevices();
    InitAddDevice();
    InitUpdateDevice();
}