#define Vec3F float4
struct QuantityInfo{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    float __no_use_padding;
    Vec3F vec;
};
#define size_cube 30
int get_idx(int x, int y, int z){
    return (((x+1)*(size_cube+1) + (y+1))*(size_cube+1) + (z+1));
}
kernel void set_quantities(global struct QuantityInfo * quant_buf){
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);
	global struct QuantityInfo * cur_info = quant_buf + get_idx(x,y,z);
    cur_info->air_mass = x/10.0f;
    cur_info->liquid_mass = y;
    //cur_info->solid_mass = z*2;
}
