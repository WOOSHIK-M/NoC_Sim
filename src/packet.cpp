#include "packet.hpp"


Packet::Packet(bool move, int x, int y, int x_chip, int y_chip)
{   
    is_move = move;

    dst_x = x;
    dst_y = y;

    dst_x_chip = x_chip;
    dst_y_chip = y_chip;
}