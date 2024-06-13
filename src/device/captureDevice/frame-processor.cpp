class FrameProcessor {
    private:
        int iterationsX;
        int iterationsY;
        double stepX;
        double stepY;
        cv::Mat newImage;
        cv::Mat processingLut;
        int paddingX;
        int paddingY;
        bool process1Pxl = false;
        bool updateStep = false;
        
        //these offset the start position of the frame mapping, allowing for padding on the edges of screens these are calculated from the bezel ratios
        double offsetSX = 0.0;
        double offsetSY = 0.0;

        double bezelScaledX = 0.0;
        double bezelScaledY = 0.0;
        //bound offset will change the bounds in which it will process the frame, mainly in use for aspect ratios, so you can take out the black bars on the top and bottom of a 16:9 video
        int boundOffsetX = 0;
        int boundOffsetY = 0;

        float deadzoneDecrease = 38.0;
        int deadzoneThreshold = 70;
        float deadzonePower = 3.0;
    
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
                postProcessing();

                if(process1Pxl){
                   cv::Scalar averageColor = cv::mean(newImage);
                }
                return true;
            }
            return false;
        }

        void postProcessing() {
             // Convert the image to HSV color space
            cv::Mat hsv;
            cv::cvtColor(newImage, hsv, cv::COLOR_BGR2HSV);
            // Split the image into H, S, and V channels
            std::vector<cv::Mat> channels;
            cv::split(hsv, channels);
            //commented out saturation for now as it isn't needed
            // // Increase the saturation channel by a certain amount
            // channels[1] += 30;
            // // Ensure that the saturation values are within the valid range
            // channels[1] = cv::min(channels[1], 255);
            // Apply a threshold to the V channel
            cv::LUT(channels[2], processingLut, channels[2]);
            // Merge the channels back into one image
            cv::merge(channels, hsv);
            // Convert the image back to BGR color space
            cv::cvtColor(hsv, newImage, cv::COLOR_HSV2BGR);
        }

        void initLut(){
                        // Define the threshold and maximum decrease
            int threshold = deadzoneThreshold;
            float maxDecrease = deadzoneDecrease;

            // Create a lookup table for the V channel adjustment
            cv::Mat lut(1, 256, CV_8U);
            uchar* p = lut.ptr();
            for (int i = 0; i < 256; ++i) {
                if (i < threshold) {
                    float factor = static_cast<float>(threshold - i) / threshold;
                    float decrease = maxDecrease * std::pow(factor, deadzonePower);
                    p[i] = static_cast<uchar>(std::max(i - static_cast<int>(decrease), 0));
                } else {
                    p[i] = i;
                }
            }
            processingLut = lut;
        }

        void processWidth(cv::Mat frame, std::string type){
            double centerX = 0.0;
            double centerY = 0.0;
            int minBoxWidth = paddingX;
            int minBoxHeight = paddingY;
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
                centerX = 0.0 - offsetSX;
                centerY = static_cast<double>(paddingY);
                sx = stepX;
                iterations = iterationsX;
            } else if(type == "bottom"){
                iOffset = iterationsX;
                centerX = 0.0 - offsetSX;
                centerY = static_cast<double>((frame.rows - 1) - paddingY);
                sx = stepX;
                iterations = iterationsX;
            } else if(type == "left"){
                iOffset = iterationsX * 2;
                centerX = static_cast<double>(paddingX);
                centerY = 0.0 - offsetSY;
                sy = stepY;
                iterations = iterationsY;
            } else if(type == "right"){
                iOffset = iterationsX * 2 + iterationsY;
                centerX = static_cast<double>((frame.cols - 1) - paddingX);
                centerY = 0.0 - offsetSY;
                sy = stepY;
                iterations = iterationsY;
            }
            int boundCol = frame.cols - 1 - boundOffsetX;
            int boundRow = frame.rows - 1 - boundOffsetY;
            for(int i = 0; i < iterations; i ++){
                if(adjustedBox){
                    boxWidth = paddingX * 2;
                    boxHeight = paddingY * 2;
                    adjustedBox = false;
                }
                x = static_cast<int>(std::floor(centerX)) - boxWidth / 2;
                y = static_cast<int>(std::floor(centerY)) - boxHeight / 2;
                //keep the box within the bounds of the frame
                if(x < boundOffsetX){
                    //bind the x value to 0 or the boundOffset if there is one, enabling aspect ratios or to add padding inside the frame and extend colors from inside edges
                    boxWidth = min(max((boxWidth + x) - boundOffsetX, minBoxWidth), boundCol);
                    x = boundOffsetX;
                    adjustedBox = true;
                } else if(x + boxWidth > boundCol){
                    //since x + boxWidth will be greater than the frame width we will subtract the difference from the boxWidth
                    boxWidth = min(max(boxWidth - ((x + boxWidth) - boundCol), boxWidth), boundCol);
                    x = boundCol - boxWidth;
                    adjustedBox = true;
                }
                if(y < boundOffsetY) {
                    boxHeight = min(max((boxHeight + y) - boundOffsetY, minBoxHeight), boundRow);
                    y=boundOffsetY;
                    adjustedBox = true;
                } else if(y + boxHeight > boundRow){
                    boxHeight = min(max(boxHeight - ((y + boxHeight) - boundRow), boxHeight), boundRow); 
                    y = boundRow - boxHeight;
                    adjustedBox = true;
                }
                //create the roi and get the average color
                cv::Rect roi(x, y, boxWidth, boxHeight);
                cv::Mat roiImage = frame(roi);

                cv::Scalar averageColor = cv::mean(roiImage); // Calculate the average color of the ROI section

                // Create a single-pixel image with the average color
                cv::Vec3b& pixel = newImage.at<cv::Vec3b>(0,i + iOffset);
                pixel = cv::Vec3b(averageColor[0], averageColor[1], averageColor[2]);

                centerX += sx;
                centerY += sy;
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
            // Darken the image
            mappingImage *= 0.8;
            return mappingImage;
        }

        void initStep(cv::Mat frame){
            //calculate the offsetSX and offsetSY
            offsetSX = bezelScaledX * static_cast<double>(frame.cols);
            offsetSY = bezelScaledY * static_cast<double>(frame.rows);
            // if(iterationsX != 0) stepX = max(std::floor((frame.cols - paddingX) / iterationsX),0);
            //sub one from the iterations as we want it to reach the edge of the frame correctly with that amount of items and starting at 0% and ending at 100%
            //offsetSX allows for padding to be computed around the edges of the frame.
            if(iterationsX != 0 && iterationsX != 1) stepX = maxd(static_cast<double>(frame.cols + (offsetSX * 2)) / static_cast<double>(iterationsX - 1),0);
            else stepX = 0.0;
            // if(iterationsY != 0) stepY = max(std::floor((frame.rows - paddingY) / iterationsY),0);
            if(iterationsY != 0 && iterationsY != 1) stepY = maxd(static_cast<double>(frame.rows + (offsetSY * 2)) / static_cast<double>(iterationsY - 1),0);
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
                    {"deadzone", {{"decrease", deadzoneDecrease}, {"threshold", deadzoneThreshold}, {"power", deadzonePower}}},
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
            if(!data["deadzone"].is_null()){
                deadzoneDecrease = data["deadzone"]["decrease"].is_null() ? deadzoneDecrease : static_cast<double>(data["deadzone"]["decrease"]);
                deadzoneThreshold = data["deadzone"]["threshold"].is_null() ? deadzoneThreshold : static_cast<int>(data["deadzone"]["threshold"]);
                deadzonePower = data["deadzone"]["power"].is_null() ? deadzonePower : static_cast<double>(data["deadzone"]["power"]);
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

        void setOffset(double x, double y){
            offsetSX = x;
            offsetSY = y;
        }

        void setBezelScaled(double x, double y){
            bezelScaledX = x;
            bezelScaledY = y;
            updateStep = true;
        }

        void setBoundOffset(int x, int y){
            boundOffsetX = x;
            boundOffsetY = y;
        }

        FrameProcessor(cv::Mat frame, int inIterationsX, int inIterationsY) {
            iterationsX = inIterationsX;
            iterationsY = inIterationsY;
            paddingX = 80;
            paddingY = 80;
            initStep(frame);
            initLut();
            newImage = cv::Mat::zeros(cv::Size(iterationsX * 2 + iterationsY * 2,1), frame.type());
          
        }
        FrameProcessor(){
            iterationsX = 10;
            iterationsY = 10;
            stepX = 0.0;
            stepY = 0.0;
            newImage = cv::Mat::zeros(cv::Size(10, 10), CV_8UC3);
            initLut();
            paddingX = 1;
            paddingY = 1;
        }
};