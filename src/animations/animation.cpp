class Animation {
    private:
        std::string name;
        std::string extension;
    public:
        cv::Mat animation;
        bool animationLoaded;

        void loadAnimation(){
            try{
            animation = cv::imread("./resources/animations/" + name + extension);
            animationLoaded = true;
            } catch(cv::Exception& e){
                std::cout << "Error loading animation " << name << " " << extension << std::endl;
            }
        }

        std::string getName() {
            return name;
        }

        std::string getExtension(){
            return extension;
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