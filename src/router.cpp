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


Router::Router(string const & topo, int _buff_size, vector<int> & _pack_lst, int normal, int mul, int idx, vector<int> & phy_x, vector<int> & phy_y, vector<int> & phy_x_chip, vector<int> & phy_y_chip, vector<vector<int>> & ChipFindIndexMap)
{   
    // Allocate Position
    _phy_x = phy_x[idx];
    _phy_y = phy_y[idx];
    _phy_x_chip = phy_x_chip[idx];
    _phy_y_chip = phy_y_chip[idx];

    RouterIdx = idx;
    ChipIdx = ChipFindIndexMap[_phy_y_chip][_phy_x_chip];

    // printf("ChipID: %i, RouterID: %i\n", ChipIdx, RouterIdx);

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

    vector<int> new_dst;
    vector<int> new_pack;

    if (normal_packets[0] != -1) {
        new_dst.emplace_back(normal_packets[0]);
        new_pack.emplace_back(1);
    }

    int dst_ptr = 1;
    while (true) {
        if (normal_packets[dst_ptr] != -1) {
            if (normal_packets[dst_ptr - 1] != normal_packets[dst_ptr]) {
                new_dst.emplace_back(normal_packets[dst_ptr]);
                new_pack.emplace_back(1);
            } else {
                new_pack.back()++;
            }
        }
        dst_ptr++;
        if (dst_ptr == (int)normal_packets.size()) {
            break;
        }
    }

    int num_packets = 0;
    if ( !new_dst.empty() ) {
        _IsActive = true;
        for (int i=0;i<(int)new_dst.size();i++) {
            _inject_lst.emplace_back(new_dst[i]);
            _inject_pack_lst.emplace_back(new_pack[i]);

            _inject_num += new_pack[i];
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
    // for (int i=0; i<(int)_multi_dst.size();i++) {
    //     cerr << _multi_dst[i];
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
    pack->last_node = RouterIdx;

    buffer->_packets.emplace_back(pack);
    buffer->_num++;

    if (buffer->_packets.size() > buff_size) {
        printf("BUFF_SIZE: %i / %i ...\n", (int)buffer->_packets.size(), buff_size);
        printf("PACKINFO: (move) = (%i) .. (dx, dy) = (%i, %i) // BUFFER ID: %i \n", (int)pack->is_move, pack->dst_x, pack->dst_y, bid);
        printf("(CORE) Invalid Routing Process ... abort ... \n");
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


// ==========================================
// ============= CHIP ROUTER ================
// ==========================================
ChipRouter::ChipRouter(string const & topo, int buff_size, int chip_latency)
{
    _buff_size = buff_size;
    _chip_latency = chip_latency;
    
    // Make Buffers
    if (topo == "mesh") {
        _num_buffer = 5;             // left, right, up, down, local
    }

    _buffers.resize(_num_buffer);
    for (int i=0;i<_num_buffer;i++) {
        _buffers[i] = new ChipBuffer(_buff_size);
    }
}


void ChipRouter::AddPacket(Packet * pack, int bid)
{
    ChipBuffer * buffer = _buffers[bid];

    pack->is_move = true;
    pack->buff_pos = bid;

    buffer->_packets.emplace_back(pack);
    buffer->_num++;

    if (buffer->_packets.size() > _buff_size) {
        // printf("PACKINFO: (move) = (%i) .. (dx, dy) = (%i, %i) // BUFFER ID: %i \n", (int)pack->is_move, pack->dst_x, pack->dst_y, bid);
        printf("(CHIP) Invalid Routing Process ... abort ... \n");
        exit(-1);
    }
}


void ChipRouter::RemovePacket(int bid)
{   
    ChipBuffer * buffer = _buffers[bid];
    
    buffer->_num--;
    buffer->_num_out++;

    buffer->_packets.pop_front();
}


void ChipRouter::Reset()
{
    for (int i=0;i<(int)_buffers.size();i++) {
        _buffers[i]->UpdateState();
    }
}