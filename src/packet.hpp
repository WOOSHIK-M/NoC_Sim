#ifndef _PACKET_HPP_
#define _PACKET_HPP_

#include <iostream>


// Packet 생성 후 return 

class Packet {
public:
    Packet(bool move, int x, int y, int _cur_node);

    int buff_pos = -1;

    bool is_move = false;

    int last_node;
    int dst_x_chip, dst_y_chip, dst_x, dst_y;

    int cross_time = 0;

    int GetRouterIdx()      { return last_node; };
};

#endif