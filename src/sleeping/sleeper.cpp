class Sleeper {
    private:
        int index = 0;
        std::string name = "default";
        std::string prev_preset = "default";

    public:
        int getIndex() {
            return index;
        }

        void setIndex(int i) {
            index = i;
        }

        std::string getName() {
            return name;
        }

        void setName(std::string n) {
            name = n;
        }

        std::string getPrevPreset() {
            return prev_preset;
        }

        void setPrevPreset(std::string p) {
            prev_preset = p;
        }

        Sleeper(int i, std::string n, std::string p) {
            index = i;
            name = n;
            prev_preset = p;

        }
};