#include "buffer.hpp"


Buffer::Buffer(int _buf_size)
{
    _buff_size = _buf_size;
}


void Buffer::CoutState()
{
    printf("A Buffer has %i buffer sizes ... \n", _buff_size);
}


int Buffer::GetBufferState()
{   
    int num_packets = _packets.size();

    if ( num_packets == _buff_size ) {      
        return 1;                           // full
    } else if ( num_packets == 0 ) {
        return -1;                           // empty
    } else {
        return 0;                           // exist
    }
}


void Buffer::RenewPacket(int pid, int new_x, int new_y)
{
    _packets[pid]->dst_x = new_x;
    _packets[pid]->dst_y = new_y;
}


void Buffer::UpdateState()
{
    _num_out = 0;
    
    for (int i=0;i<(int)_packets.size();i++) {
        _packets[i]->is_move = false;
    }

    if (_packets.size() == _buff_size) {
        _buff_full = true;
        _buff_empty = false;
    } else if (_packets.empty()) {
        _buff_full = false;
        _buff_empty = true;
    } else {
        _buff_full = false;
        _buff_empty = false;
    }
}