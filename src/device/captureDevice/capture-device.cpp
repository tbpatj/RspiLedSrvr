#include "./frame-processor.cpp"

class CaptureDevice {
    private:
        cv::VideoCapture cap;
        cv::Mat frame;
        int capFPS = 30;
        FrameProcessor frameProcessor;
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
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if( cv::waitKey (30) >= 0) return;
            updateFrame();
            
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

        void processFrame() {
            //x direction
            frameProcessor.process(frame);
            
            cv::imshow("New Image", frameProcessor.getImage());
        
        }

// void processFrame(int resolution) {
//     float scaleFactor = resolution / 100.0f;
//     float step = 1.0f / scaleFactor;
//     int width = static_cast<int>(frame.cols * scaleFactor);
//     int height = static_cast<int>(frame.rows * scaleFactor);
//     cv::Mat newImage = cv::Mat::zeros(cv::Size(width, height), frame.type());
//     int padding = std::floor(step);
//     std::cout << "width: " << width << " height: " << height << " step: " << step << " padding: " << padding << std::endl;
//     int x = padding;
//     int y = padding;

//     for(int i = 0; i < 10; i ++){
//         cv::Rect roi(static_cast<int>(x + (i * step) - padding), y - padding, padding, padding);
//         cv::Mat roiImage = frame(roi);

//         cv::Mat blurredImage;
//         cv::boxFilter(roiImage, blurredImage, -1, cv::Size(padding, padding));

//         blurredImage.copyTo(newImage(roi));
//     }
//      cv::imshow("New Image", newImage);
   
// }
        
        
        // void processFrame(){
        //     // Define the region of interest (x, y, width, height)
        //     int x = 100;
        //     int y = 100;
        //     int width = 200;
        //     int height = 200;

        //     // Create a sub-image using the specified region of interest
        //     cv::Mat roi = frame(cv::Rect(x, y, width, height));

        //     // Create a kernel for averaging
        //     cv::Mat kernel = cv::Mat::ones(width, height, CV_32F) / (width * height);

        //     // Apply the convolution operation to calculate the average pixel value
        //     cv::Mat result;
        //     cv::filter2D(roi, result, -1, kernel);

        //     // Display the result
        //     cv::imshow("Average", result);
        // }
                    
        

        CaptureDevice(){
            try{
                initCapture();
                updateFrame();
                frameProcessor = FrameProcessor(frame, 10, 10);
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
            }
        }
};