#include <algorithm>

#include "router.hpp"

using namespace std;


template<typename T>
std::vector<T> slice(std::vector<T> const &v, int m, int n)
{
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n + 1;
 
    std::vector<T> vec(first, last);
    return vec;
}


Router::Router(string const & topo, int _buff_size, vector<int> & _pack_lst, int normal, int mul, int idx, vector<int> & phy_x, vector<int> & phy_y)
{   
    // Allocate Position
    _phy_x = phy_x[idx];
    _phy_y = phy_y[idx];

    // Make Buffers
    if (topo == "mesh") {
        _num_buffer = 5;             // left, right, up, down, local
    }

    buff_size = _buff_size;
    _buffers.resize(_num_buffer);
    for (int i=0;i<_num_buffer;i++) {
        _buffers[i] = new Buffer(_buff_size);
    }

    // Make Inject Packet list
    vector<int> normal_packets = slice(_pack_lst, 0, normal - 1);
    vector<int> normal_packets_copy = normal_packets;
    
    auto last_normal = unique(normal_packets.begin(), normal_packets.end());
    normal_packets.erase(last_normal, normal_packets.end());

    int num_packets = 0;
    for (int i=0;i<(int)normal_packets.size();i++) {
        if (normal_packets[i] != -1) {
            _IsActive = true;

            _inject_lst.emplace_back(normal_packets[i]);           // Destination

            num_packets = count(normal_packets_copy.begin(), normal_packets_copy.end(), normal_packets[i]);
            _inject_num += num_packets;
            _inject_pack_lst.emplace_back(num_packets);            // Packet Numbers
        }
    }

    // Make Multicast destination list
    vector<int> multi_packets = slice(_pack_lst, normal, mul - 1);
    auto last_mul = unique(multi_packets.begin(), multi_packets.end());
    multi_packets.erase(last_mul, multi_packets.end());

    for (int i=0;i<(int)multi_packets.size();i++) {
        if (multi_packets[i] != -1) {
            _is_mul = true;
            _multi_dst.emplace_back(multi_packets[i]);           // Destination
            
            mul_x = phy_x[multi_packets[0]];
            mul_y = phy_y[multi_packets[0]];
        }
    }

    // ============= Print Traffic Information =============
    // printf("\n");
    // printf("normal destination : ");
    // for (int i=0; i<(int)_inject_lst.size();i++) {
    //     cerr << _inject_lst[i] << " ";
    // }
    // cerr << endl;
    // printf("normal destination packets : ");
    // for (int i=0; i<(int)_inject_pack_lst.size();i++) {
    //     cerr << _inject_pack_lst[i] << " ";
    // }
    // cerr << endl;
    // printf("is multicast node? : ");
    // cerr << _is_mul << " (";
    // for (int i=0; i<(int)multi_packets.size();i++) {
    //     cerr << multi_packets[i] << " ";
    // }
    // cerr << ")";
    // cerr << endl << "=============================== \n";
}


void Router::CoutState()
{
    printf("A Router has %i buffers ... \n", (int)_buffers.size());
    _buffers[0]->CoutState();
}


void Router::InjectPacket()
{   
    _inject_pack_lst[0]--;

    if (_inject_pack_lst[0] == 0) {
        _inject_lst.pop_front();
        _inject_pack_lst.pop_front();
    }
}


Packet * Router::RemovePacket(int bid, int pid)
{   
    Buffer * buffer = _buffers[bid];
    
    buffer->_num--;
    buffer->_num_out++;

    Packet * pack = buffer->_packets[pid];
    buffer->_packets.erase(buffer->_packets.begin() + pid);      // Remove packet
    
    return pack;
}


void Router::AddPacket(Packet * pack, int bid)
{
    Buffer * buffer = _buffers[bid];

    pack->is_move = true;
    pack->buff_pos = bid;
    buffer->_packets.emplace_back(pack);
    buffer->_num++;

    if (buffer->_packets.size() > buff_size) {
        // printf("PACKINFO: (move) = (%i) .. (dx, dy) = (%i, %i) // BUFFER ID: %i \n", (int)pack->is_move, pack->dst_x, pack->dst_y, bid);
        printf("Invalid Routing Process ... abort ... \n");
        exit(-1);
    }
}


void Router::ArrivedPacket(int bid, int pid)
{
    Buffer * buffer = _buffers[bid];
    
    buffer->_num--;
    buffer->_num_out++;

    delete buffer->_packets[pid];                                       // Free Memory
    buffer->_packets.erase(buffer->_packets.begin() + pid);      // Remove packet
}


void Router::Reset()
{
    local_in_out = 0;

    for (int i=0;i<(int)_buffers.size();i++) {
        _buffers[i]->UpdateState();
    }
}


void Router::PrintState()
{
    for (int i=0;i<(int)_buffers.size();i++) {
        printf("Buffer: %i ( size: %i)\n", i, (int)_buffers[i]->_packets.size());
    }
}