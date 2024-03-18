class LedController {
    protected:

    public:
        virtual ~LedController() = default;
        virtual bool initLed(int pinOut, int ledCount) {
            return false;
        }
        virtual void setLEDColor(int index, int color)=0;
        virtual void setLEDColor(int index, int r, int g, int b)=0;
        virtual void render()=0;
};