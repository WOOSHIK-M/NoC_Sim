#include "packet.hpp"


Packet::Packet(bool move, int x, int y)
{   
    is_move = move;
    dst_x = x;
    dst_y = y;
}