#include "./frame-processor.cpp"

class CaptureDevice {
    private:
        cv::VideoCapture cap;
        cv::Mat frame;
        int capFPS = 30;
        int numEmptyFrames = 0;
        FrameProcessor frameProcessor;
    public:
        bool isCapturing = false;

        void initCapture(){
            try{
                numEmptyFrames = 0;
                // mac open webcam code \/
                // cap.open(0,cv::CAP_AVFOUNDATION);
                cap.open(-1);
                //check to make sure that the capture was actually opened
                if(!cap.isOpened()){
                    isCapturing = false;
                    std::cerr << "Error: couldn't open the webcam." << std::endl;
                    return;
                }
                std::cout << "Webcam opened successfully" << std::endl;
                //if the capture was opened make sure to set what it captures at in terms of framerate.
                isCapturing = true;
                cap.set(cv::CAP_PROP_FPS, capFPS);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if( cv::waitKey (30) >= 0) return;
                updateFrame();
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
                 if(!cap.isOpened()){
                    isCapturing = false;
                 }
            }
            
        }
        bool isWindowClosed() {
            return false;
            // return cv::getWindowProperty("Webcam", cv::WND_PROP_VISIBLE) == 0;
        }
        
        void show() {
            cv::imshow("Webcam", frame);
        }

        cv::Mat getImage() {
            return frameProcessor.getImage();
        }


        void updateFrame(){
            if(isCapturing){
                cap >> frame;
            }
        }

        void processFrame() {
            //x direction
            if(isCapturing){
                bool isValidStep = frameProcessor.process(frame);
                //debug for the webcam feed
                if(show_webcam_feed) cv::imshow("Webcam", frame);
                
                if(isValidStep){
                    numEmptyFrames = 0;
                    //debug for the processed image
                    if(show_processed_image) cv::imshow("New Image", frameProcessor.getImage());
                    
                } else {
                    //if the frame size is actually bigger than 0 than lets try to reinitialize the frame processor
                    if(frame.size().width != 0 && frame.size().height != 0){
                        frameProcessor.initStep(frame);
                    }
                    numEmptyFrames ++;
                    std::cout << "\ncurrent empty frame count: " << numEmptyFrames << "\n" << std::endl;
                    if(numEmptyFrames > 10){
                        std::cerr << "Error: too many empty frames" << std::endl;
                        isCapturing = false;
                    }
                }
            }
        }

        json getCaptureMappings(){
            return frameProcessor.getCaptureMappings();
        }

        CaptureDevice(){
            try{
                initCapture();
                updateFrame();
                frameProcessor = FrameProcessor(frame, 16, 9);
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
            }
        }
};