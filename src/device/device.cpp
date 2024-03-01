class Device {
    protected:
        std::string name;
    public:
        virtual ~Device() = default;
        virtual void update() = 0;
        virtual json getJson() = 0;
        virtual void setData( json data) = 0;

        virtual void setName(std::string name) {
            this->name = name;
        };
        virtual std::string getName() {
            return name;
            };
        // common methods for all devices
};