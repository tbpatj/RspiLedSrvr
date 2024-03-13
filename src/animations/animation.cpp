class Animation {
    private:
        std::string name;
    public:
        std::string extension;
        cv::Mat animation;
        bool animationLoaded;

        void loadAnimation(){
            animation = cv::imread("path/to/" + name + extension);
            animationLoaded = true;
        }

        std::string getName() {
            return name;
        }

        cv::Mat getAnimation() {
            if(!animationLoaded){
                loadAnimation();
            }
            return animation;
        }

        Animation(){
            name = "default";
            extension = ".png";
        }

        Animation(std::string nName, std::string nExtension){
            name = nName;
            extension = nExtension;
            animationLoaded = false;
            std::cout << "Animation " << name << " " << extension << " loaded" << std::endl;
        }
};