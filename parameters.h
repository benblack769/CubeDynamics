#pragma once
// important mechanical constants, feel free to fiddle with these
#define seconds_per_calc 0.0001f
#define attraction_force_coef 50000.0f
#define liquid_pressure_coef 0.8f
#define solid_pressure_coef 0.5f
#define gass_pressure_coef 10.0f
#define gravity_constant 1000.0f

#define size_cube 100


//programatic constants, do not change without carefully examining code useage
#define SIDES_ON_CUBE 6

#define SIZE_FOLD 4
#define NUM_FOLDS (size_cube/SIZE_FOLD)
