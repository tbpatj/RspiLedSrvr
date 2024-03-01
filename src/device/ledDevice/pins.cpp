class LedDevicePins {
    private:
        int pin;
        int pin1;
        int pin2;

    public:

    LedDevicePins(){
        pin = 0;
        pin1 = 0;
        pin2 = 0;
    }

    void setData(json data) {
        pin = data["r"].is_null() ? pin : data["r"].get<int>();
        pin1 = data["g"].is_null() ? pin1 : data["g"].get<int>();
        pin2 = data["b"].is_null() ? pin2 : data["b"].get<int>();
    }

    void setData(int inPin){
        pin = inPin;
    }

    json getNonAddressableJson() {
        json data = {
            {"r", pin},
            {"g", pin1},
            {"b", pin2}
        };
        return data;
    }

    int getAddressablePinout(){
        return pin;
    }

    LedDevicePins(int inPin) {
        pin = inPin;
    }

    LedDevicePins(int pin0, int inPin1, int inPin2){
        pin = pin0;
        pin1 = inPin1;
        pin2 = inPin2;
    }

};