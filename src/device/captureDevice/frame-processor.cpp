class FrameProcessor {
    private:
        int iterationsX;
        int iterationsY;
        double stepX;
        double stepY;
        cv::Mat newImage;
        int paddingX;
        int paddingY;
        bool process1Pxl = false;
        bool updateStep = false;
    
    public:
        //returns true if was able to actually process the frame

        //v2 processing
        bool process(cv::Mat frame){
            if(updateStep) {
                updateStep = false;
                initStep(frame);
            }
            if(write_frame_proccessor_data) std::cout << "Processing Frame\n frame width: " << frame.size().width << " frame height: " << frame.size().height << "\niterationsX: " << iterationsX << " iterationsY: " << iterationsY << "\npaddingX: " << paddingX << " paddingY: " << paddingY << "\nstepX: " << stepX << " stepY: " << stepY << " New Image: " << newImage.size().width << " " << newImage.size().height << std::endl;
            if(stepX != 0 && stepY != 0 && frame.cols > 0 && frame.rows > 0){
                processWidth(frame, "top");
                processWidth(frame, "bottom");
                processWidth(frame, "left");
                processWidth(frame, "right");

                
                // Convert the image to HSV color space
                cv::Mat hsv;
                cv::cvtColor(newImage, hsv, cv::COLOR_BGR2HSV);
                // Split the image into H, S, and V channels
                std::vector<cv::Mat> channels;
                cv::split(hsv, channels);
                // Increase the saturation channel by a certain amount
                channels[1] += 30;
                // Ensure that the saturation values are within the valid range
                channels[1] = cv::min(channels[1], 255);
                // Apply a threshold to the V channel
                cv::Mat mask;
                cv::threshold(channels[2], mask, 100, 255, cv::THRESH_BINARY_INV);
               // Create a copy of the V channel
                cv::Mat vDecreased = channels[2].clone();

                // Decrease the V channel by a certain amount
                vDecreased -= 30;

                // Use the mask to apply the decrease only to the darker colors
                cv::bitwise_and(vDecreased, mask, vDecreased);
                cv::bitwise_and(channels[2], ~mask, channels[2]);
                channels[2] += vDecreased;
                // Merge the channels back into one image
                cv::merge(channels, hsv);
                // Convert the image back to BGR color space
                cv::cvtColor(hsv, newImage, cv::COLOR_HSV2BGR);

                


                if(process1Pxl){
                   cv::Scalar averageColor = cv::mean(newImage);
                }
                return true;
            }
            return false;
        }

        void processWidth(cv::Mat frame, std::string type){
            double centerX = 0.0;
            double centerY = 0.0;
            int boxWidth = paddingX * 2;
            int boxHeight = paddingY * 2;
            bool adjustedBox = false;
            int x = 0;
            int y = 0;
            int iterations = 0;
            int iOffset = 0;
            double sx = 0;
            double sy = 0;
            if(type == "top"){
                iOffset = 0;
                centerX = 0.0;
                centerY = static_cast<double>(paddingY);
                sx = stepX;
                iterations = iterationsX;
            } else if(type == "bottom"){
                iOffset = iterationsX;
                centerX = 0.0;
                centerY = static_cast<double>((frame.rows - 1) - paddingY);
                sx = stepX;
                iterations = iterationsX;
            } else if(type == "left"){
                iOffset = iterationsX * 2;
                centerX = static_cast<double>(paddingX);
                centerY = 0.0;
                sy = stepY;
                iterations = iterationsY;
            } else if(type == "right"){
                iOffset = iterationsX * 2 + iterationsY;
                centerX = static_cast<double>((frame.cols - 1) - paddingX);
                centerY = 0.0;
                sy = stepY;
                iterations = iterationsY;
            }
            int boundCol = frame.cols - 1;
            int boundRow = frame.rows - 1;
            for(int i = 0; i < iterations; i ++){
                if(adjustedBox){
                    boxWidth = paddingX * 2;
                    boxHeight = paddingY * 2;
                    adjustedBox = false;
                }
                x = static_cast<int>(std::floor(centerX)) - boxWidth / 2;
                y = static_cast<int>(std::floor(centerY)) - boxHeight / 2;
                //keep the box within the bounds of the frame
                if(x < 0){
                    //since x will be negative we will subtract the boxwidth from it so that it crops the roi to the edge of the frame
                    boxWidth += x;
                    x = 0;
                    adjustedBox = true;
                } else if(x + boxWidth > boundCol){
                    //since x + boxWidth will be greater than the frame width we will subtract the difference from the boxWidth
                    boxWidth -= (x + boxWidth) - boundCol;
                    x = boundCol - boxWidth;
                    adjustedBox = true;
                }
                if(y < 0) {
                    boxHeight += y;
                    y=0;
                    adjustedBox = true;
                } else if(y + boxHeight > boundRow){
                    boxHeight -= (y + boxHeight) - boundRow;
                    y = boundRow - boxHeight;
                    adjustedBox = true;
                }
                //create the roi and get the average color
                cv::Rect roi(x, y, boxWidth, boxHeight);
                cv::Mat roiImage = frame(roi);

                cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

                // Create a single-pixel image with the average color
                cv::Vec3b& pixel = newImage.at<cv::Vec3b>(0,i + iOffset);
                pixel[0] = averageColor[0]; // Blue channel
                pixel[1] = averageColor[1]; // Green channel
                pixel[2] = averageColor[2]; // Red channel

                centerX += sx;
                centerY += sy;
            }
        }



        // bool process(cv::Mat frame){
        //     if(updateStep) {
        //         updateStep = false;
        //         initStep(frame);
        //     }
        //     if(write_frame_proccessor_data) std::cout << "Processing Frame\n frame width: " << frame.size().width << " frame height: " << frame.size().height << "\niterationsX: " << iterationsX << " iterationsY: " << iterationsY << "\npaddingX: " << paddingX << " paddingY: " << paddingY << "\nstepX: " << stepX << " stepY: " << stepY << " New Image: " << newImage.size().width << " " << newImage.size().height << std::endl;
        //     if(stepX != 0 && stepY != 0){
        //         //process each side of the frame
        //         //new image is returned on one row in success after each other
        //         //top edge
        //         getBlurredLength(frame,iterationsX, 1, 0, 0, paddingX, 0);
        //         // bottom edge
        //         getBlurredLength(frame,iterationsX, 1, 0, 1, paddingX, iterationsX);
        //         // left edge
        //         getBlurredLength(frame,iterationsY, 0, 1, 0, paddingY, iterationsX * 2);
        //         // right edge
        //         getBlurredLength(frame,iterationsY, 0, 1, 1, paddingY, iterationsX * 2 + iterationsY);
        //         if(process1Pxl){
        //            cv::Scalar averageColor = cv::mean(newImage);
        //         }
                
        //         return true;
        //     } else {
        //         return false;
        //     }
            
        // }


        // void getBlurredLength(cv::Mat frame,int iterations,int xStep,int yStep,bool addBase,int padding, int offset){
        //     int x = addBase && xStep == 0 ? frame.size().width - padding : padding;
        //     int y = addBase && yStep == 0 ? frame.size().height - padding : padding;
        //     if(frame.cols > 0 && frame.rows > 0){
        //         for(int i = 0; i < iterations; i ++){
        //             cv::Rect roi(static_cast<int>(x + (i * stepX * xStep) - padding), static_cast<int>(y + (i * stepY * yStep) - padding), padding, padding);
        //             cv::Mat roiImage = frame(roi);

        //             cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

        //             // Create a single-pixel image with the average color
        //             cv::Vec3b& pixel = newImage.at<cv::Vec3b>(0,i + offset);
        //             pixel[0] = averageColor[0]; // Blue channel
        //             pixel[1] = averageColor[1]; // Green channel
        //             pixel[2] = averageColor[2]; // Red channel
        //         }
        //     }
        // }

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
            // Darken the image
            mappingImage *= 0.8;
            return mappingImage;
        }

        void initStep(cv::Mat frame){
            // if(iterationsX != 0) stepX = max(std::floor((frame.cols - paddingX) / iterationsX),0);
            //sub one from the iterations as we want it to reach the edge of the frame correctly with that amount of items and starting at 0% and ending at 100%
            if(iterationsX != 0 && iterationsX != 1) stepX = maxd(static_cast<double>(frame.cols) / static_cast<double>(iterationsX - 1),0);
            else stepX = 0.0;
            // if(iterationsY != 0) stepY = max(std::floor((frame.rows - paddingY) / iterationsY),0);
            if(iterationsY != 0 && iterationsY != 1) stepY = maxd(static_cast<double>(frame.rows) / static_cast<double>(iterationsY - 1),0);
            else stepY = 0.0;
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
            stepX = 0.0;
            stepY = 0.0;
            newImage = cv::Mat::zeros(cv::Size(10, 10), CV_8UC3);
            paddingX = 1;
            paddingY = 1;
        }
};