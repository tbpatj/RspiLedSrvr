class NonAddressableController {
    //default for a non addressable controller is an RGB strip. not RGBW or RGBWW
    protected:
        std::vector<int> pins;
        std::vector<int> color;
        int count;
    public:
        //resize the pin count to 3
        NonAddressableController(){
            pins.clear();
            color.clear();
            for(int i = 0; i < 3; i ++){
                pins.push_back(0);
                color.push_back(0);
            }
        };

        // maybe don't use this as it could have unforseen effects, like if they accidentally set the pin to one being used.
        // void setDefaultPinsMode() {
        //     gpioSetMode(pins[0], PI_INPUT);
        //     gpioSetMode(pins[1], PI_INPUT);
        //     gpioSetMode(pins[2], PI_INPUT);
        // }

        virtual void setPins(std::vector<int> inPins) {
            pins = inPins;
        }
        virtual void setPins(json data){
            // setDefaultPinsMode();
            pins[0] = data["r_pin"].is_null() ? pins[0] : static_cast<int>(data["r_pin"]);
            pins[1] = data["g_pin"].is_null() ? pins[1] : static_cast<int>(data["g_pin"]);
            pins[2] = data["b_pin"].is_null() ? pins[2] : static_cast<int>(data["b_pin"]);
            if(gpio_initialised){
                // gpioSetMode(pins[0], PI_OUTPUT);
                // gpioSetMode(pins[1], PI_OUTPUT);
                // gpioSetMode(pins[2], PI_OUTPUT);
            }
         }

        virtual void setColor(std::vector<int> inColor) {
            color = inColor;
        }

        //when needed for other types of strips this can be overridden
        virtual void setColor(int r, int g, int b){
            color[0] = r;
            color[1] = g;
            color[2] = b;
            if(gpio_initialised){
                for(int i = 0; i < 3; i ++){
                    // gpioPWM(pins[i], color[i]);
                }
            }
        }

        virtual void render() {
            //render the color to the strip
        }
        
        virtual json getPinsJson() {
            json data = {
                {"r_pin", pins[0]},
                {"g_pin", pins[1]},
                {"b_pin", pins[2]}
            };
            return data;
        } 
        std::vector<int> getPins() {
            return pins;
        }
};