#pragma once
#include "quantity_info.h"
#include <vector>
#include <mutex>

class CubeSharedData;

void update_data_continuously(CubeSharedData * data_to_update);

class CubeSharedData{
private:
    std::vector<QuantityInfo> cube_data;
    std::mutex lock;
public:
    CubeSharedData(size_t size);
    void read_into(std::vector<QuantityInfo> & cube_buffer);
    void write(std::vector<QuantityInfo> & buffer);
};

