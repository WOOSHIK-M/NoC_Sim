#include "config_data.hpp"

ConfigData::ConfigData()
{
	// ======= LUT Options ========
	AddStrField("LUT_file", "");
	_int_map["lut_normal"] = 256;
	_int_map["lut_mul"] = 512;

	// ======= Topology Options =======
	AddStrField("topology", "mesh");
	_int_map["x_dim"] = 8;			// number of routers in a chip
	_int_map["y_dim"] = 8;
	_int_map["X_DIM"] = 1;			// number of chips			
	_int_map["Y_DIM"] = 1;

	// ======== Routing Options =======
	_int_map["multicast_mode"] = 1;
	AddStrField("routing_function", "none");
	AddStrField("init_place", "zigzag");
	_int_map["buff_size"] = 2;
	_int_map["chip_buff_size"] = 10;
	_int_map["vir_chan"] = 1;
	_int_map["interval"] = 0;
}
