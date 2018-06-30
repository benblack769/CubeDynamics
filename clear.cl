kernel void set_output_error(global float * clear_buf){
	uint j = get_global_id(0);
	clear_buf[j] = -1;
}
