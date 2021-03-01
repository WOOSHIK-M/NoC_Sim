#ifndef _INIT_PLACE_HPP_
#define _INIT_PLACE_HPP_

#include <map>
#include <vector>

using namespace std;


typedef void (* initPtr)(vector<int> & x, vector<int> & y, vector<int> & x_chip, vector<int> & y_chip, \
                        int _nodes, int _nodesPerChip, int x_dim, int y_dim, int X_DIM, int Y_DIM, \
                        vector<vector<int>> & FindIndexMap, vector<vector<int>> & ChipFindIndexMap);

void InitializeInitMap();

extern map<string, initPtr> InitFunction;

#endif