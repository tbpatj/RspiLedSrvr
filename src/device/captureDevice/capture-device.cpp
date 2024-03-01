class CaptureDevice {
    private:
        cv::VideoCapture cap;
        cv::Mat frame;
        int capFPS = 30;
    public:
        bool isCapturing = false;

        void initCapture(){
            cap.open(0, cv::CAP_AVFOUNDATION);
            //check to make sure that the capture was actually opened
            if(!cap.isOpened()){
                isCapturing = false;
                std::cerr << "Error: couldn't open the webcam." << std::endl;
                return;
            }
            //if the capture was opened make sure to set what it captures at in terms of framerate.
            isCapturing = true;
            cap.set(cv::CAP_PROP_FPS, capFPS);
        }
        bool isWindowClosed() {
            return false;
            // return cv::getWindowProperty("Webcam", cv::WND_PROP_VISIBLE) == 0;
        }
        
        void show() {
            cv::imshow("Webcam", frame);
            
        }

        void updateFrame(){
            if(isCapturing){
                cap >> frame;
            }
        }

        CaptureDevice(){
            try{
                initCapture();
                updateFrame();
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
            }
        }
};