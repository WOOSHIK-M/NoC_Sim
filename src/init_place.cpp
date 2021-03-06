#include <iostream>
#include <map>
#include <vector>

#include "init_place.hpp"

using namespace std;

void zigzag(vector<int> & phy_x, vector<int> & phy_y, vector<int> & x_chip, vector<int> & y_chip, \
            int _nodes, int _nodesPerChip, int x_dim, int y_dim, int X_DIM, int Y_DIM, \
            vector<vector<int>> & FindIndexMap, vector<vector<int>> & ChipFindIndexMap)
{
    phy_x.resize(_nodes);
    phy_y.resize(_nodes);

    x_chip.resize(_nodes);
    y_chip.resize(_nodes);

	int posx = 0;
	int posy = 0;
	int posX = 0;
	int posY = 0;

	int index = 0;
    int chipidx = 0;

    int rx = 0;
    int ry = 0;

    while (posY < Y_DIM) {
        while (posX < X_DIM) {
            while (posy < y_dim) {
                while (posx < x_dim) {
                    rx = posx + x_dim*posX;
                    ry = posy + y_dim*posY;

                    phy_x[index] = rx;
                    phy_y[index] = ry;

                    x_chip[index] = posX;
                    y_chip[index] = posY;
   
                    FindIndexMap[ry][rx] = index;
                    ChipFindIndexMap[posY][posX] = chipidx;

                    index++;
                    posx++;
                }
                posy += 1;
                posx = 0;
            }
            chipidx++;
            posX += 1;
            posy = 0;
        }
        posY += 1;
        posX = 0;
    }
}


map<string, initPtr> InitFunction;
void InitializeInitMap() {
    InitFunction["zigzag"]           = &zigzag;
}