class TransitionObject {
    private:
        //transition variables used for when a preset changes
        std::chrono::milliseconds tStart;
        float t = 0.0f;
        long long transitionSpeed = 1000;
    public: 

        TransitionObject(){
            t=0.0f;
            tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        }
        TransitionObject(long long speed){
            t=0.0f;
            transitionSpeed = speed;
            tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        }
        void setTransitionSpeed(long long speed){
            transitionSpeed = speed;
        }
        void setData(json data){
            transitionSpeed = data["transition_speed"].is_null() ? transitionSpeed : static_cast<long long>(data["transition_speed"]);
        }
        long long getTransitionSpeed(){
            return transitionSpeed;
        }
        void resetTiming() {
            //reset the actual timing percentage
            t = 0;
            //reset the clock timer to the time now
            tStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        }

        void updateTiming() {
            //if the timing object is less than one then we are still transitioning so keep updating the percentage of the transition
            if(t < 1.0f){
                //get the current time since a specific time a while ago...
                std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                //get the difference in time elapsed. Then divide it by the amount of time we are trying to transition for. it will be above 1 if we are done transitioning
                t = static_cast<float>((now - tStart).count()) / transitionSpeed;
                //just making sure to clip the value to 1 if it goes over
                if(t >= 1.0f) t = 1.0f;
            }
        }
        float getPercT() {
            return t;
        }
        void setPercT(float perc) {
            t = perc;
        }

    
};