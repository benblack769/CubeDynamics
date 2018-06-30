#pragma once
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include "cube_coords.h"
using namespace std;

void cell_update_main_loop();

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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void rendered(){
        prev_time = chrono::system_clock::now();
    }
    double duration_since_render(){
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


class RenderBufferData{
    //Assumes that the same thread does not call update_check and write values.
protected:
    bool has_written = false;
    mutex has_written_lock;
    vector<BYTE> cube_colors;
    vector<float> cube_verticies;
    vector<BYTE> write_cube_colors;
    vector<float> write_cube_verticies;
public:
    bool update_check(){
        if(has_written_lock.try_lock()){
            if(has_written){
                has_written = false;

                cube_colors.swap(write_cube_colors);
                cube_verticies.swap(write_cube_verticies);
            }
            has_written_lock.unlock();
            return true;
        }
        else{
            return false;
        }
    }
    vector<BYTE> & get_colors(){
        return cube_colors;
    }
    vector<float> & get_verticies(){
        return cube_verticies;
    }
    void set_vals(vector<BYTE> & colors,vector<float> & verticies){
        if(has_written_lock.try_lock()){
            has_written = false;
            has_written_lock.unlock();
        }
        write_cube_colors = colors;
        write_cube_verticies = verticies;
        has_written_lock.lock();
        has_written = true;
        has_written_lock.unlock();
    }
};

extern RenderBufferData all_buffer_data;
