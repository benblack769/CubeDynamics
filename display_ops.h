#pragma once
#include "quantity_info.h"
#include "cube_color.h"
#include "cube_coords.h"

QuantityInfo random_init();
RGBVal color(QuantityInfo info);
bool is_transparent(QuantityInfo info);

std::vector<FaceDrawInfo> get_exposed_faces(QuantityInfo * all_data);
void update(QuantityInfo *source_data, QuantityInfo * update_data);
std::vector<QuantityInfo> create_quantity_data_vec();
std::vector<LargeVec> create_random_vecs();
