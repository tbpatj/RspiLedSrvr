class Device {
    public:
        virtual ~Device() = default;
        virtual void update() = 0;
        virtual void getJson() = 0;
        virtual void setData( json data) = 0;
        // common methods for all devices
};