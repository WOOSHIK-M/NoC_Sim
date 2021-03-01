#include <iostream>
#include <map>

#include "routing_func.hpp"

using namespace std;


int XY(int src_x, int src_y, int dst_x, int dst_y)          // Get direction? or next node index? 
{
    if (src_x > dst_x) {
        return 1;
    } else if (src_x < dst_x) {
        return 2;
    } else {
        if (src_y > dst_y) {
            return 3;
        } else if (src_y < dst_y) {
            return 4;
        } else {
            return 0;
        }
    }
}


map<string, routingPtr> routingFunction;
void InitializeRoutingMap() {
    routingFunction["XY"]           = &XY;
}