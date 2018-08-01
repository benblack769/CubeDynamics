#pragma once
#include <chrono>
#include <thread>

class FrameRateControl{
    double desired_framerate;
    std::chrono::system_clock::time_point prev_time;
public:
    FrameRateControl(double in_desired_framerate){
        desired_framerate = in_desired_framerate;
       // prev_time = chrono::system_clock::now();
    }
    void render_pause(){
        while(!this->should_render()){
            this->spin_sleep();
        }
        this->rendered();
    }
    void spin_sleep(){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    void rendered(){
        prev_time = std::chrono::system_clock::now();
    }
    double duration_since_render(){
        using namespace std;
        using namespace chrono;
        system_clock::time_point cur_time = system_clock::now();

        auto duration = duration_cast<std::chrono::milliseconds>(cur_time - prev_time);

        const int MIL_PER_SEC = 1000;

        return duration.count()/double(MIL_PER_SEC);
    }
    bool should_render(){
        return duration_since_render() > 1.0/desired_framerate;
    }
};
