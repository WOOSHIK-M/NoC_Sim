#include <iostream>
#include <vector>

#include <chrono>

#include "config_data.hpp"
#include "config_utils.hpp"
#include "network.hpp"
#include "routing_func.hpp"
#include "init_place.hpp"

using namespace std;


int main() 
{
    string ConfigPath = "config/config.ini";

    InitializeRoutingMap();                 // Initialize Routing Function
    InitializeInitMap();                    // Initialize Mapping Function

    ConfigData config;                      // Build Configuration Class

    ParseArgs( &config, ConfigPath );       // Parsing Configuration Data
    
    int LUT_NUM = config.GetInt("lut_mul");
	int NODE_NUM = config.GetInt("X_DIM") * config.GetInt("Y_DIM") * config.GetInt("x_dim") * config.GetInt("y_dim");
    string lutname = string("data") + SLASH + config.GetStr("LUT_file");

    vector<vector<int>> LUT(NODE_NUM, vector<int>(LUT_NUM, -1));
    ParseLUTs( LUT, lutname );                // Parsing Look Up Table

    Network net( &config, LUT );                    // Initialize Network  
    net.CoutState();

    chrono::system_clock::time_point routing_start, routing_end;
	routing_start = chrono::system_clock::now();

    net.Simulate();

    routing_end = chrono::system_clock::now();
	auto pass = chrono::duration_cast<chrono::milliseconds>(routing_end - routing_start).count();
	cout<<"Simlation Finished in "<< pass <<" ms."<<endl;

    return 0;
}