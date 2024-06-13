#include "./frame-processor.cpp"

class CaptureDevice {
    private:
        cv::VideoCapture cap;
        cv::Mat frame;
        int capFPS = 30;
        int numEmptyFrames = 0;
        FrameProcessor frameProcessor;
        int last_frame_checks = 1;
        
        int aspectW = 0;
        int aspectH = 0;
        int screenTruthW = 16;
        int screenTruthH = 9;

        //bezel is a percentage of how big the bezel is in comparision to the screen width and height
        double bezelX = 0.0;
        double bezelY = 0.0;
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
                cap.open(0);
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

        cv::Mat getRawFrame() {
            return frame;
        }

        cv::Mat getMappingsImage() {
            return frameProcessor.getMappingsImage();
        }


        void updateFrame(){
            now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            if(isCapturing){
                cap >> frame;
                checkLastFrames();
                checkSleepTime();
            }
        }

        bool checkRatio(double curRatio, double w, double h) {
            double aspectRatio = w / h;
            if(abs(curRatio - aspectRatio) < 0.05){
                aspectW = static_cast<int>(w);
                aspectH = static_cast<int>(h);
                return true;
            }
            return false;
        }

        void calcAspectRatio(){
            int cols = frame.cols;
            int rows = frame.rows;
            //default to 16 / 9
            aspectW = 16;
            aspectH = 9;
            double ratio = static_cast<double>(cols) / static_cast<double>(rows);
            checkRatio(ratio, 21, 3);
            checkRatio(ratio, 1, 1);
            checkRatio(ratio, 4, 3);
            checkRatio(ratio, 16, 10);
            checkRatio(ratio, 32, 9);
        }

        void setAspectRatio(int widthA, int heightA){
            int cols = frame.cols;
            int rows = frame.rows;
            aspectW = widthA;
            aspectH = heightA;
            //get the aspect ratio of the current input
            //create a truth aspect ratio, as the current input frame may be incorrect. We may be stretching the image to fit the screen
            double screenTruthRatio = 16.0/9.0;
            int truthCols = rows * screenTruthRatio;
            int truthRows = cols / screenTruthRatio;
            // float aspectRatio = static_cast<float>(cols) / static_cast<float>(rows);
            double nAR = static_cast<double>(widthA) / static_cast<double>(heightA);
            // std::cout << "screentruth " << screenTruthRatio << " Truth Cols: " << truthCols << " Truth Rows: " << truthRows << " nAR " << nAR << " Capture Cols: " << cols << " Capture Rows: " << rows << "\n" << std::endl;
            if(nAR > screenTruthRatio){
                //if the new aspect ratio is wider than the current aspect ratio then we need to get the amount of pixels for the black bar
                int newRows = truthCols / nAR;
                int diff = (rows - newRows) / 2;
                frameProcessor.setBoundOffset(0, diff);
            } else if(nAR < screenTruthRatio){
                //if the new aspect ratio is taller than the current aspect ratio then we need to crop the top and bottom
                int newCols = truthRows * nAR;
                int diff = (cols - newCols) / 2;
                // std::cout << "diff " << diff << " newCols " << newCols << std::endl;
                frameProcessor.setBoundOffset(diff, 0);
            } else {
                frameProcessor.setBoundOffset(0, 0);
            }
        }

        void calcBezel(){
            int cols = frame.cols;
            int rows = frame.rows;
            double truthAspectRatio = static_cast<double>(screenTruthW) / static_cast<double>(screenTruthH);
            double captureAspectRatio = static_cast<double>(cols) / static_cast<double>(rows);
            if(truthAspectRatio > captureAspectRatio){
                int nCols = rows * truthAspectRatio;
                double diff = static_cast<double>(cols) / static_cast<double>(nCols);
                double nBezelX = bezelX * diff;
                frameProcessor.setBezelScaled(nBezelX, bezelY);
            } else if(truthAspectRatio < captureAspectRatio){
                int nRows = cols / truthAspectRatio;
                double diff = static_cast<double>(rows) / static_cast<double>(nRows);
                double nBezelY = bezelY * diff;
                frameProcessor.setBezelScaled(bezelX, nBezelY);
            }
        }

        void setScreenTruth(int w, int h){
            screenTruthW = w;
            screenTruthH = h;
            calcBezel();
            setAspectRatio(aspectW,aspectH);
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
                
                if(isValidStep){
                    numEmptyFrames = 0;
                    
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

        json getJson() {
            json out = frameProcessor.getJson();
            out["input"] = {{ "width", frame.cols }, { "height", frame.rows } };
            out["aspect_ratio"] = std::to_string(aspectW) + ":" + std::to_string(aspectH);
            out["truth_aspect_ratio"] = {{"x", screenTruthW}, {"y", screenTruthH}};
            out["bezel"] =  {{"x", bezelX},{"y", bezelY}};
            return out;
        }

        void setData(json data) {
             if(!data["truth_aspect_ratio"].is_null()){
                screenTruthW = data["truth_aspect_ratio"]["x"].is_null() ? screenTruthW : static_cast<int>(data["truth_aspect_ratio"]["x"]);
                screenTruthH = data["truth_aspect_ratio"]["y"].is_null() ? screenTruthH : static_cast<int>(data["truth_aspect_ratio"]["y"]);
                setScreenTruth(screenTruthW, screenTruthH);
                if(aspectW == 0 && aspectH == 0){
                    setAspectRatio(screenTruthW,screenTruthH);
                }
            }
            if(!data["bezel"].is_null()){
                std::cout << "bezel change" << std::endl;
                bezelX = data["bezel"]["x"].is_null() ? bezelX : static_cast<double>(data["bezel"]["x"]);
                bezelY = data["bezel"]["y"].is_null() ? bezelY : static_cast<double>(data["bezel"]["y"]);
                calcBezel();
            }
            frameProcessor.setData(data);
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