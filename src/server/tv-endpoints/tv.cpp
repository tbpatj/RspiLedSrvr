#include "./get-tv-settings.cpp"
#include "./save-tv-settings.cpp"
#include "./tv-aspect-ratio.cpp"

void InitTVEndpoints() {
    InitGetTVSettings();
    InitSaveTVSettings();
    InitTVAspectRatio();
}