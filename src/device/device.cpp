class Device {
    protected:
        std::string name;
        std::string preset = "default";
        //0 for non-addressable LED
        //1 for addressable LED
        int type;
    public:
        virtual ~Device() = default;
        virtual void update() = 0;
        virtual json getJson() = 0;
        virtual void setData( json data) = 0;
        virtual void setPreset(std::string presetName){
            this->preset = presetName;
        }

        virtual std::string getPreset() {
            return preset;
        }

        virtual void setName(std::string name) {
            this->name = name;
        };
        virtual std::string getName() {
            return name;
        };

        virtual int usingTV(){
            return 0;
        }

        //type declarations
        virtual int getIntType(){
            return type;
        }
        virtual void setType(int nType) {
            this->type = nType;
        }
         virtual void setType(std::string nType) {
            if(nType == "addressable"){
                this->type = 1;
            } else if(nType == "non-addressable"){
                this->type = 0;
            }
            
        }
        virtual std::string getStrType(){
            if(type == 1){
                return "addressable";
            } else if(type == 0){
                return "non-addressable";
            }
            return "none";
        }

        // common methods for all devices
};