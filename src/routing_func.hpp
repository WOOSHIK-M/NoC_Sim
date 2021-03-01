#ifndef _ROUTING_FUNC_HPP_
#define _ROUTING_FUNC_HPP_

#include <map>
#include <vector>

#include "packet.hpp"

using namespace std;


typedef int (* routingPtr)(int src_x, int src_y, int dst_x, int dst_y);

void InitializeRoutingMap();

extern map<string, routingPtr> routingFunction;

#endif