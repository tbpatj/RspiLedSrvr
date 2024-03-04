class FrameProcessor {
    private:
        int iterationsX;
        int iterationsY;
        int stepX;
        int stepY;
        cv::Mat newImage;
        int paddingX;
        int paddingY;
    
    public:
        //returns true if was able to actually process the frame
        bool process(cv::Mat frame){
            if(write_frame_proccessor_data) std::cout << "Processing Frame\n frame width: " << frame.size().width << " frame height: " << frame.size().height << "\niterationsX: " << iterationsX << " iterationsY: " << iterationsY << "\npaddingX: " << paddingX << " paddingY: " << paddingY << "\nstepX: " << stepX << " stepY: " << stepY << std::endl;
            if(stepX != 0 && stepY != 0){
                //process each side of the frame
                getBlurredLength(frame,iterationsX, 1, 0, 0, paddingX);
                getBlurredLength(frame,iterationsY, 0, 1, 0, paddingY);
                getBlurredLength(frame,iterationsX, 1, 0, 1, paddingX);
                getBlurredLength(frame,iterationsY, 0, 1, 1, paddingY);
                return true;
            } else {
                return false;
            }
            
        }


        void getBlurredLength(cv::Mat frame,int iterations,int xStep,int yStep,bool addBase,int padding){
            int x = addBase && xStep == 0 ? frame.size().width - padding : padding;
            int y = addBase && yStep == 0 ? frame.size().height - padding : padding;

            int basePxlX = addBase && xStep == 0 ? iterationsX - 1 : 0;
            int basePxlY = addBase && yStep == 0 ? iterationsY - 1 : 0;
            // int i=0;
            for(int i = 0; i < iterations; i ++){
                cv::Rect roi(static_cast<int>(x + (i * stepX * xStep) - padding), static_cast<int>(y + (i * stepY * yStep) - padding), padding, padding);
                cv::Mat roiImage = frame(roi);

                cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

                // Create a single-pixel image with the average color
                cv::Vec3b& pixel = newImage.at<cv::Vec3b>(i * yStep + basePxlY,i * xStep + basePxlX);
                pixel[0] = averageColor[0]; // Blue channel
                pixel[1] = averageColor[1]; // Green channel
                pixel[2] = averageColor[2]; // Red channel
            }
        }

        void initStep(cv::Mat frame){
            stepX = max(std::floor((frame.cols - paddingX) / iterationsX),0);
            stepY = max(std::floor((frame.rows - paddingY) / iterationsY),0);
        }

        cv::Mat getImage() {
            return newImage;
        }

        FrameProcessor(cv::Mat frame, int inIterationsX, int inIterationsY) {
            iterationsX = inIterationsX;
            iterationsY = inIterationsY;
            paddingX = 80;
            paddingY = 80;
            initStep(frame);
            newImage = cv::Mat::zeros(cv::Size(iterationsX,iterationsY), frame.type());
          
        }
        FrameProcessor(){
            iterationsX = 10;
            iterationsY = 10;
            stepX = 0;
            stepY = 0;
            newImage = cv::Mat::zeros(cv::Size(10, 10), CV_8UC3);
            paddingX = 1;
            paddingY = 1;
        }
};