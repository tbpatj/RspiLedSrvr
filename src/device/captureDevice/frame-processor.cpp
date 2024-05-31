class FrameProcessor {
    private:
        int iterationsX;
        int iterationsY;
        int stepX;
        int stepY;
        cv::Mat newImage;
        int paddingX;
        int paddingY;
        bool process1Pxl = false;
        bool updateStep = false;
    
    public:
        //returns true if was able to actually process the frame
        bool process(cv::Mat frame){
            if(updateStep) {
                updateStep = false;
                initStep(frame);
            }
            if(write_frame_proccessor_data) std::cout << "Processing Frame\n frame width: " << frame.size().width << " frame height: " << frame.size().height << "\niterationsX: " << iterationsX << " iterationsY: " << iterationsY << "\npaddingX: " << paddingX << " paddingY: " << paddingY << "\nstepX: " << stepX << " stepY: " << stepY << " New Image: " << newImage.size().width << " " << newImage.size().height << std::endl;
            if(stepX != 0 && stepY != 0){
                //process each side of the frame
                //new image is returned on one row in success after each other
                //top edge
                getBlurredLength(frame,iterationsX, 1, 0, 0, paddingX, 0);
                // bottom edge
                getBlurredLength(frame,iterationsX, 1, 0, 1, paddingX, iterationsX);
                // left edge
                getBlurredLength(frame,iterationsY, 0, 1, 0, paddingY, iterationsX * 2);
                // right edge
                getBlurredLength(frame,iterationsY, 0, 1, 1, paddingY, iterationsX * 2 + iterationsY);
                if(process1Pxl){
                   cv::Scalar averageColor = cv::mean(newImage);
                }
                
                return true;
            } else {
                return false;
            }
            
        }
        json getCaptureMappings(){
            json captureMappings = {
                {"topS", 0}, // Add a new key-value pair to the JSON object
                {"topE", iterationsX - 1},
                {"bottomS", iterationsX},
                {"bottomE", iterationsX * 2 - 1},
                {"leftS", iterationsX * 2},
                {"leftE", iterationsX * 2 + iterationsY - 1},
                {"rightS", iterationsX * 2 + iterationsY},
                {"rightE", iterationsX * 2 + iterationsY * 2 - 1}
                };
            return captureMappings;
        }

        cv::Mat getMappingsImage(){
            cv::Mat mappingImage = cv::Mat::zeros(cv::Size(iterationsX * 2 + iterationsY * 2,1), CV_8UC3);
            //draw the line for the top edge, Red
            cv::line(mappingImage, cv::Point(0, 0), cv::Point(iterationsX - 1, 0), cv::Scalar(144, 1, 245), 1);
            //draw the line for the bottom edge Yellow
            cv::line(mappingImage, cv::Point(iterationsX, 0), cv::Point(iterationsX * 2 - 1, 0), cv::Scalar(255, 1, 120), 1);
            //draw the line for the left edge
            cv::line(mappingImage, cv::Point(iterationsX * 2, 0), cv::Point(iterationsX * 2 + iterationsY - 1, 0), cv::Scalar(1, 0, 142), 1);
            //draw the line for the right edge Blue
             cv::line(mappingImage, cv::Point(iterationsX * 2 + iterationsY, 0), cv::Point(iterationsX * 2 + iterationsY * 2 - 1, 0), cv::Scalar(254, 0, 234), 1);
            // Blur the image
            cv::GaussianBlur(mappingImage, mappingImage, cv::Size(21, 1), 0);
            // Darken the image
            mappingImage *= 0.8;
            return mappingImage;
        }

        void getBlurredLength(cv::Mat frame,int iterations,int xStep,int yStep,bool addBase,int padding, int offset){
            int x = addBase && xStep == 0 ? frame.size().width - padding : padding;
            int y = addBase && yStep == 0 ? frame.size().height - padding : padding;
            if(frame.cols > 0 && frame.rows > 0){
                for(int i = 0; i < iterations; i ++){
                    cv::Rect roi(static_cast<int>(x + (i * stepX * xStep) - padding), static_cast<int>(y + (i * stepY * yStep) - padding), padding, padding);
                    cv::Mat roiImage = frame(roi);

                    cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

                    // Create a single-pixel image with the average color
                    cv::Vec3b& pixel = newImage.at<cv::Vec3b>(0,i + offset);
                    pixel[0] = averageColor[0]; // Blue channel
                    pixel[1] = averageColor[1]; // Green channel
                    pixel[2] = averageColor[2]; // Red channel
                }
            }
        }

        void initStep(cv::Mat frame){
            if(iterationsX != 0) stepX = max(std::floor((frame.cols - paddingX) / iterationsX),0);
            else stepX = 0;
            if(iterationsY != 0) stepY = max(std::floor((frame.rows - paddingY) / iterationsY),0);
            else stepY = 0;
        }

        cv::Mat getImage() {
            return newImage;
        }

        json getJson() {
            json data = {
                    {"padding", {{"x", paddingX},{"y", paddingY}}},
                    {"iterations", {{"x", iterationsX},{"y", iterationsY}}},
                    {"step", {{"x", stepX},{"y", stepY}}},
                    {"process1Pxl", process1Pxl}
                };
            return data;
        }
        void setData(json data) {
            // paddingX
            if(!data["padding"].is_null()){
                paddingX = data["padding"]["x"].is_null() ? paddingX : static_cast<int>(data["padding"]["x"]);
                paddingY = data["padding"]["y"].is_null() ? paddingY : static_cast<int>(data["padding"]["y"]);
            }
            if(!data["iterations"].is_null()){
                iterationsX = data["iterations"]["x"].is_null() ? iterationsX : static_cast<int>(data["iterations"]["x"]);
                iterationsY = data["iterations"]["y"].is_null() ? iterationsY : static_cast<int>(data["iterations"]["y"]);
                newImage = cv::Mat::zeros(cv::Size(iterationsX * 2 + iterationsY * 2,1), CV_8UC3);
            }
            updateStep = true;
            // name = data["name"].is_null() ? (name.empty() ? "default" : name) : static_cast<std::string>(data["name"]);
            // deviceType = data["device_type"].is_null() ?  deviceType : data["device_type"] == "addressable" ? 1 : 0;
            // mode = data["mode"].is_null() ?  mode : getModeFromStr(static_cast<std::string>(data["mode"]));
            // power = data["power"].is_null() ? power : data["power"] == "on" ? true : false;
            // deviceName = data["device_name"].is_null() ? (deviceName) : static_cast<std::string>(data["device_name"]);
            // //update timings
            // animSpeed = data["animation_speed"].is_null() ? animSpeed : static_cast<long long>(data["animation_speed"]);
            // setMappings(data);
        }

        FrameProcessor(cv::Mat frame, int inIterationsX, int inIterationsY) {
            iterationsX = inIterationsX;
            iterationsY = inIterationsY;
            paddingX = 80;
            paddingY = 80;
            initStep(frame);
            newImage = cv::Mat::zeros(cv::Size(iterationsX * 2 + iterationsY * 2,1), frame.type());
          
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