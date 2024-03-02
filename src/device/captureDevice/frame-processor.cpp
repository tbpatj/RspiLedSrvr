class FrameProcessor {
    private:
        int iterationsX;
        int iterationsY;
        float stepX;
        float stepY;
        cv::Mat newImage;
        int paddingX;
        int paddingY;
    
    public:
        void process(cv::Mat frame){
            std::cout << "Processing Frame\n frame width: " << frame.size().width << " frame height: " << frame.size().height << "\niterationsX: " << iterationsX << " iterationsY: " << iterationsY << "\npaddingX: " << paddingX << " paddingY: " << paddingY << std::endl;
            getBlurredLength(frame,iterationsX, stepX, 0, paddingX);
            getBlurredLength(frame,iterationsY, 0, stepY, paddingY);
        }

        void getBlurredLength(cv::Mat frame,int iterations,float xStep,float yStep,int padding){
            int x = padding + 1;
            int y = padding + 1;
            for(int i = 0; i < iterations - 1; i ++){
                cv::Rect roi(static_cast<int>(x + (i * xStep) - padding), static_cast<int>(y + (i * yStep) - padding), padding, padding);
                cv::Mat roiImage = frame(roi);

                cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

                // Create a single-pixel image with the average color
                cv::Vec3b& pixel = newImage.at<cv::Vec3b>(i,0);
                pixel[0] = averageColor[0]; // Blue channel
                pixel[1] = averageColor[1]; // Green channel
                pixel[2] = averageColor[2]; // Red channel
            }
        }

        cv::Mat getImage() {
            return newImage;
        }

        FrameProcessor(cv::Mat frame, int inIterationsX, int inIterationsY) {
            iterationsX = inIterationsX;
            iterationsY = inIterationsY;
            stepX = frame.cols / iterationsX;
            stepY = frame.rows / iterationsY;
            newImage = cv::Mat::zeros(cv::Size(iterationsX,iterationsY), frame.type());
            paddingX = std::floor(stepX);
            paddingY = std::floor(stepY);
          
        }
        FrameProcessor(){
            iterationsX = 10;
            iterationsY = 10;
            stepX = 1;
            stepY = 1;
            newImage = cv::Mat::zeros(cv::Size(10, 10), CV_8UC3);
            paddingX = 1;
            paddingY = 1;
        }
};