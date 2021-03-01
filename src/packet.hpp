#ifndef _PACKET_HPP_
#define _PACKET_HPP_

#include <iostream>


// Packet 생성 후 return 

class Packet {
public:
    Packet(bool move, int x, int y);

    int buff_pos = -1;
    bool is_move = false;
    int dst_x, dst_y;
};

#endif