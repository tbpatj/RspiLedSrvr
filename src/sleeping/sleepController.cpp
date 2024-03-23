#include "./sleeper.cpp"

class SleepController {
    std::vector<Sleeper> sleepers;
    int state = 0;
    std::chrono::milliseconds last_check = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    public: 
    void update() {
        std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        //check every 2 seconds if the tv signal is still lost or the picture is still sleeping
        if((now - last_check).count() > 2000 && !using_webcam){
            last_check = now;
            //check if the tv signal is lost
            if(tv_no_signal || tv_sleeping){
                captureDevice.updateFrame();
            }
        } else if(using_webcam) {
            last_check = now;
        }
        //when there is no tv signal and we haven't slept all the devices, go through and sleep all devices
        if(tv_no_signal && state == 0){
            std::cout << "Sleeping" << std::endl;
            state = 1;
            json sleepPreset = {{"preset", "sleep"}};
            for(int i = 0; i < devices.size(); i ++){
                std::string name = devices[i]->getName();
                std::string preset = devices[i]->getPreset();
                int usesTV = devices[i]->usingTV();
                if(preset != "custom" && usesTV == 1){
                    sleepers.push_back(Sleeper(i, name, preset));
                    devices[i]->setData(sleepPreset);
                }
            }
        }
        //when there is a tv signal and we have slept all the devices, wake all the devices
        if(!tv_no_signal && state == 1){
            std::cout << "Waking" << std::endl;
            state = 0;
            for(int i = 0; i < sleepers.size(); i++){
                int index = sleepers[i].getIndex();
                std::string preset = sleepers[i].getPrevPreset();
                json lastPreset = {{"preset", preset}};
                std::string currentPreset = devices[index]->getPreset();
                if(currentPreset == "sleep"){
                    devices[index]->setData(lastPreset);
                }
            }
            sleepers.clear();
        }

    }
};