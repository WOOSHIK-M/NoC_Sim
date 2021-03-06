#include "packet.hpp"


Packet::Packet(bool move, int x, int y, int _cur_node)
{   
    is_move = move;

    dst_x = x;
    dst_y = y;

    last_node = _cur_node;
}