#include "./frame-processor.cpp"

class CaptureDevice {
    private:
        cv::VideoCapture cap;
        cv::Mat frame;
        int capFPS = 30;
        int numEmptyFrames = 0;
        FrameProcessor frameProcessor;
        int last_frame_checks = 1;
        //time since last frame change
        std::chrono::milliseconds timeSLFC;
        std::chrono::milliseconds now;

        std::vector<cv::Vec3b> no_capture_pixels = {
            cv::Vec3b(255,255,255),
            cv::Vec3b(0,255,255),
            cv::Vec3b(255,255,0),
            cv::Vec3b(0,255,0),
            cv::Vec3b(255, 0, 255),
            cv::Vec3b(0, 0, 255),
            cv::Vec3b(255, 0, 0),
            cv::Vec3b(0,0,0)};

        long long sleepTime = 3000;

        std::vector<cv::Vec3b> last_frame_pixels;
        std::vector<cv::Vec2i> last_frame_pixel_locations;
    public:
        bool isCapturing = false;

        void initCapture(){
            try{
                numEmptyFrames = 0;
                // mac open webcam code \/
                cap.open(0,cv::CAP_AVFOUNDATION);
                // cap.open(-1);
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
                updateLastFrameChecks(1);
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
            now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            if(isCapturing){
                cap >> frame;
                checkLastFrames();
                checkSleepTime();
            }
        }

        void checkSleepTime(){
            if(static_cast<float>((now - timeSLFC).count()) > sleepTime){
                tv_sleeping = true;
            } else {
                tv_sleeping = false;
            }
        }
        
        bool checkIfSignal(int index){
            int randomX =  last_frame_pixel_locations[index][0];
            cv::Vec3b framePixel = last_frame_pixels[index];
            float sectionDec = static_cast<float>(randomX) / static_cast<float>(frame.cols) * 8.0f;
            int sectionF = static_cast<int>(std::floor(sectionDec));
            int sectionC = static_cast<int>(std::ceil(sectionDec));
            //check if it's on the borders of the color barriers, if so there tends to be some anti-aliasing that messes with things
            if( std::abs(sectionDec - sectionF) > 0.1 && std::abs(sectionC - sectionDec) > 0.1){
                //if the section matches the current pixels frame then we know there could potentially be no signal
                if(no_capture_pixels[sectionF] == framePixel){
                    //iterate through all the different sections to see if we are actually on the no signal screen
                    for(int section = 0; section < 8; section++){
                        //get the position of the section
                        int posX = static_cast<int>(std::floor(((static_cast<float>(section) + 0.5f) / 8.0f) * frame.cols));
                        cv::Vec3b framePixel = frame.at<cv::Vec3b>(30, posX);
                        if(framePixel != no_capture_pixels[section]){
                            tv_no_signal = false;
                            return false;
                        }
                    }
                    tv_no_signal = true;
                    return true;
                    // std::cout << " stage " << section << " red " << static_cast<int>(framePixel[2]) << " green " << static_cast<int>(framePixel[1]) << " blue " << static_cast<int>(framePixel[0]) <<  std::endl;
                }
                tv_no_signal = false;
            }
            return false;
        }

        //uses a specified number of probs to see if the last frame is the same as the current frame.
        void checkLastFrames(){
            if(frame.cols > 0 && frame.rows > 0){
                for (int i = 0; i < last_frame_checks; i++) {
                    cv::Vec2i location = last_frame_pixel_locations[i];
                    cv::Vec3b pixel = last_frame_pixels[i];

                    if (location[0] >= 0 && location[0] < frame.cols && location[1] >= 0 && location[1] < frame.rows) {
                        cv::Vec3b framePixel = frame.at<cv::Vec3b>(location[1], location[0]);
                        // std::cout << "locationX: " << location[0] << " locationY: " << location[1] << " olPixelR: " << static_cast<int>(pixel[0]) << " newPixelR: " << static_cast<int>(framePixel[0]) << std::endl;

                        if (framePixel != pixel) {
                            timeSLFC = now;
                        }
                        last_frame_pixels[i] = framePixel;
                    }
                    int randomX = std::floor(rand() % frame.cols);
                    int randomY = std::floor(rand() % frame.rows);
                    location = cv::Vec2i(randomX, randomY);
                    last_frame_pixel_locations[i] = location;
                    cv::Vec3b framePixel = frame.at<cv::Vec3b>(location[1], location[0]);
                    last_frame_pixels[i] = framePixel;

                    if (location[0] >= 0 && location[0] < frame.cols && location[1] >= 0 && location[1] < frame.rows) {
                    //detection to see if the frame is (at least for my capture card) the image where no signal is captured
                        checkIfSignal(i);
                    }
                }
            }
            
        }

        void updateLastFrameChecks(int checks){
            last_frame_checks = checks;
            now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            timeSLFC = now;
            last_frame_pixels.clear();
            last_frame_pixel_locations.clear();
            for (int i = 0; i < checks; i++) {
                last_frame_pixel_locations.push_back(cv::Vec2i(0, 0));
                last_frame_pixels.push_back(cv::Vec3b(0,0,0));
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