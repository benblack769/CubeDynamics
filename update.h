#pragma once
#include "quantity_info.h"

void update_coord_quantity(QuantityInfo * source_data, float * source_bonds, QuantityInfo * update_data, float * all_exchange_data, CubeCoord base_coord);
void update_bonds(QuantityInfo * source_data, QuantityInfo * updated_data, float * source_bonds, float * all_exchange_data, float * update_bonds, CubeCoord base_coord);
