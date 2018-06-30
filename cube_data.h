#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include "cube_info.h"
#include <cstdlib>
#include <vector>

const int size_cube = 30;

std::vector<FaceDrawInfo> get_exposed_faces(QuantityInfo * all_data);
void update(QuantityInfo *source_data, QuantityInfo * update_data);
QuantityInfo * create_data();
