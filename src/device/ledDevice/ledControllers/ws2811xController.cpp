// #include <ws2811.h>

class WS2811xController : public LedController {
    private:
        // ws2811_channel_t* channel;
        // ws2811_t ledstring;
    public:
        WS2811xController(int pinOut, int ledCount){
            // ledstring = {};
            // ledstring.freq = WS2811_TARGET_FREQ;
            // ledstring.dmanum = 10; // Choose a DMA channel

            // channel = &ledstring.channel[0];
            // channel->gpionum = pinOut;
            // channel->count = ledCount;
            // channel->invert = 0;
            // channel->brightness = 255;
            // channel->strip_type = WS2811_STRIP_GRB; // Set the correct color order for WS2812B

            // if (ws2811_init(&ledstring)) {
            //     std::cerr << "ws2811_init failed" << std::endl;
            // }
        }
        bool initLed(int pinOut, int ledCount) override{
            return false;
        }
        void setLEDColor(int index, int color) override{
            // channel->leds[index] = color;
        };
        void setLEDColor(int index, int r, int g, int b) override {
            // channel->leds[index] =  r << 16 | g << 8 | b;
        };
        void render() override {
            // ws2811_render(&ledstring);
        };

};
