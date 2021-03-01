#include <iostream>
#include <map>
#include <vector>

#include "init_place.hpp"

using namespace std;

void zigzag(vector<int> & phy_x, vector<int> & phy_y, int _nodes, int x_dim, int y_dim, int X_DIM, int Y_DIM, vector<vector<int>> & FindIndexMap)
{
    phy_x.resize(_nodes);
    phy_y.resize(_nodes);

	int posx = 0;
	int posy = 0;
	int posX = 0;
	int posY = 0;

	int index = 0;

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
   
                    FindIndexMap[ry][rx] = index;
                    index++;
                    posx++;
                }
                posy += 1;
                posx = 0;
            }
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