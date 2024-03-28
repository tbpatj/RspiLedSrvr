#include "./add-device.cpp"
#include "./update-device.cpp"
#include "./get-devices.cpp"
#include "./delete-device.cpp"

void InitDeviceEndpoints() {
    InitGetDevices();
    InitAddDevice();
    InitUpdateDevice();
    InitDeleteDevice();
}