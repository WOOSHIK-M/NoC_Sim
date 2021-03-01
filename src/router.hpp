#ifndef _ROUTER_HPP_
#define _ROUTER_HPP_

#include <iostream>
#include <vector>

#include "config_data.hpp"
#include "buffer.hpp"

using namespace std;


enum {localinpos = -1, localpos, leftpos, rightpos, uppos, downpos};

class Router
{
private:
    bool _IsActive = false;

    int _phy_x, _phy_y;

    int _num_buffer;
    int buff_size;
    vector<Buffer *> _buffers;          //local, left, right, up, down
    
    int _inject_num = 0;
    // deque<int> _inject_lst;
    // deque<int> _inject_pack_lst;
    
    bool _is_mul = false;
    vector<int> _multi_dst;

    int mul_x, mul_y = -1;

public:
    deque<int> _inject_lst {};
    deque<int> _inject_pack_lst {};

    int _time = 0;
    int local_in_out = 0;

    Router(string const & topo, int _buff_size, vector<int> & _pack_lst, int normal, int mul, int idx, vector<int> & phy_x, vector<int> & phy_y);

    void CoutState();

    bool IsActive()         { return _IsActive; };

    int GetInjectNum()      { return _inject_num; };
    int GetNumBuffer()      { return _num_buffer; };

    int GetNewDest()        { return _inject_lst[0]; };
    auto GetDestCoor(int bid, int pid) { return _buffers[bid]->GetDestCoor(pid); };
    
    bool GetInjectState()   { return _inject_lst.empty(); };

    bool IsMul() { return _is_mul; };

    bool GetIsMove(int bid, int pid)    {return _buffers[bid]->_packets[pid]->is_move; };

    bool GetLocalIsFull()   { return _buffers[localpos]->GetIsFull(); };
    bool GetLeftIsFull()    { return _buffers[leftpos]->GetIsFull(); };
    bool GetRightIsFull()   { return _buffers[rightpos]->GetIsFull(); };
    bool GetUpIsFull()      { return _buffers[uppos]->GetIsFull(); };
    bool GetDownIsFull()    { return _buffers[downpos]->GetIsFull(); };

    bool GetInjectIsEmpty()  { return _inject_lst.empty(); };

    bool GetLocalIsEmpty()   { return _buffers[localpos]->GetIsEmpty(); };
    bool GetLeftIsEmpty()    { return _buffers[leftpos]->GetIsEmpty(); };
    bool GetRightIsEmpty()   { return _buffers[rightpos]->GetIsEmpty(); };
    bool GetUpIsEmpty()      { return _buffers[uppos]->GetIsEmpty(); };
    bool GetDownIsEmpty()    { return _buffers[downpos]->GetIsEmpty(); };

    int GetLocalState()     { return _buffers[localpos]->GetBufferState(); };

    void RenewPacket(Packet * pack) { pack->dst_x = mul_x; pack->dst_y = mul_y; };      // multicast

    int GetNum(int bid)     { return _buffers[bid]->_num; };
    int GetNumOut(int bid)  { return _buffers[bid]->_num_out; };
   
    void InjectPacket();       // local_in to channel
    Packet * GetPacketAddress(int bid, int pid) { return _buffers[bid]->_packets[pid]; };

    Packet * RemovePacket(int bid, int pid);
    void AddPacket(Packet * pack, int bid);
    void ArrivedPacket(int bid, int pid);

    void Reset();

    void PrintState();
};

#endif


