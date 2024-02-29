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

    LedDevicePins(int inPin) {
        pin = inPin;
    }

    LedDevicePins(int pin0, int inPin1, int inPin2){
        pin = pin0;
        pin1 = inPin1;
        pin2 = inPin2;
    }

};