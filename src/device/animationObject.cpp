class AnimationObject {
    private:
        int animIndx = 0;
        std::chrono::milliseconds aStart;
        float animT = 0.0f;
        cv::Mat animImage;
        int cCols = 0;
        
    public: 
        void setAnimIndx(int indx){
            animIndx = indx;
        }

        void setAnimImage(cv::Mat image){
            animImage = image;
        }

        int getCCols(){
            return cCols;
        }  

        cv::Mat interpolateFrames(const cv::Mat& curFrame, const cv::Mat& nextFrame, float fraction) {
            cv::Mat interpolatedFrame;
            cv::addWeighted(curFrame, 1 - fraction, nextFrame, fraction, 0, interpolatedFrame);
            return interpolatedFrame;
        }

        cv::Mat getCurFrame() {
            if(animImage.cols > 0 && animImage.rows > 0 && animIndx < animImage.rows){
                cv::Rect roi(0,animIndx,animImage.cols,1);
                cv::Mat cFrame = animImage(roi);
                //get next frame
                int ceilFrameIndx = animIndx + 1;
                if(ceilFrameIndx >= animImage.rows) ceilFrameIndx = 0;
                //if the next frame is different than the current frame then interpolate between the two frames
                if(ceilFrameIndx != animIndx){
                    cv::Rect roi2(0,ceilFrameIndx,animImage.cols,1);
                    cv::Mat ceilFrame = animImage(roi2);

                    //interpolate between the two frames so we are at the current frame
                    cFrame = interpolateFrames(cFrame, ceilFrame, animT);
                }
                cCols = animImage.cols;
                if(show_animation) cv::imshow("Animation", cFrame);
                return cFrame;
            } else {
                cv::Mat blackImage(1, 1, CV_8UC3, cv::Scalar(0, 0, 0));
                return blackImage;
            }
        }

        void resetTiming(int animI) {
            animImage = animations[animI].getAnimation();
            //if the new animation that is loaded up is smaller than the last then we will need to move the current frame potentially.
            if(animIndx >= animImage.rows) {
                animT = 0;
                std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                // aStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                aStart = now;
                animIndx = 0;
            }
        }

         //here is where we will perform certain operations on the image of the animation to make it so the animation changes or something
        void updateAnimationAfterLooped() {
            // Call the colorShift function with the desired shift value
            // colorShift(animImage, 30); // Shift the hue by 30 degrees 
        }

        void colorShift(cv::Mat& image, int shift) {
            cv::Mat hsvImage;
            cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

            // Shift the hue channel
            hsvImage.forEach<cv::Vec3b>([shift](cv::Vec3b& pixel, const int* position) {
                pixel[0] = (pixel[0] + shift) % 180; // Wrap around the hue values
            });

            cv::cvtColor(hsvImage, image, cv::COLOR_HSV2BGR);
        }

        void updateTiming(long long animSpeed) {
            std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            animT = static_cast<float>((now - aStart).count()) / animSpeed;
            
            //if animation should move to next frame move it.
            if(animT >= 1){
                //reset the start time
                animT = 1.0f - min(animT,2.0f);
                // aStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                std::chrono::milliseconds animSpeedMs(static_cast<long long>(animSpeed * animT));
                aStart = now - animSpeedMs;

                //make sure to add an if to check if it should move to the next frame or if it needs to repeat

                //move the current frame of the animation to the next
                animIndx++;
                if(animIndx >= animImage.rows) {
                    updateAnimationAfterLooped();
                    animIndx = 0;
                }
            }
        }
        
    
};