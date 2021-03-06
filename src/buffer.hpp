#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_

#include <iostream>
#include <deque>
#include <tuple>

#include "config_data.hpp"
#include "packet.hpp"

using namespace std;


class Buffer {

private:
    int _buff_size;

    bool _buff_full = false, _buff_empty = true;

public:
    vector<Packet *> _packets;

    int _num = 0;
    int _num_out = 0;

    Buffer(int _buff_size);

    void CoutState();

    int GetBufferState();           // 0-full, 1-empty, 2-packet exist

    bool GetIsEmpty()   { return _buff_empty; };
    bool GetIsFull()    { return _buff_full; };

    void UpdateState();

    void RecievePacket(Packet const * packet) { };

    void RemovePacket(int pid) { _packets.erase(_packets.begin() + pid); };

    void RenewPacket(int pid, int new_x, int new_y);

    auto GetDestCoor(int pid) { return tuple<int, int>(_packets[pid]->dst_x, _packets[pid]->dst_y); };
};


class ChipBuffer {

private:
    int _buff_size;

    bool _buff_full = false, _buff_empty = true;

public:
    deque<Packet *> _packets;

    int _num = 0;
    int _num_out = 0;

    ChipBuffer(int _buff_size);

    bool GetIsEmpty()   { return _buff_empty; };
    bool GetIsFull()    { return _buff_full; };

    void UpdateState();
};

#endif