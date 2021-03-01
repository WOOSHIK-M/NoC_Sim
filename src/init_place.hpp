#ifndef _INIT_PLACE_HPP_
#define _INIT_PLACE_HPP_

#include <map>
#include <vector>

using namespace std;


typedef void (* initPtr)(vector<int> & x, vector<int> & y, int _nodes, int x_dim, int y_dim, int X_DIM, int Y_DIM, vector<vector<int>> & FindIndexMap);

void InitializeInitMap();

extern map<string, initPtr> InitFunction;

#endif