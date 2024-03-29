#ifndef _NETWORK_HPP_
#define _NETWORK_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include "config_data.hpp"
#include "router.hpp"
#include "packet.hpp"
#include "routing_func.hpp"
#include "init_place.hpp"

using namespace std;


class Network 
{
private:
    int normal, mul;

    int x_dim, y_dim, X_DIM, Y_DIM;
    int _nodes, _chips, _nodesPerChip;
    int _num_buffer;
    int _buff_size;

    int _vir_chan, _interval;
    
    int _chip_buff_size, _interchip_latency;

    string _topology;
    string _parseip;
    string _parserf;
    
    initPtr ip;
    routingPtr rf;

    vector<int> phy_x;
    vector<int> phy_y;
    vector<int> phy_x_chip;
    vector<int> phy_y_chip;

    vector<vector<int>> FindIndexMap;
    vector<vector<int>> ChipFindIndexMap;

    vector<vector<int>> _net;
    vector<vector<int>> _chip_net;

    vector<vector<bool>> _inuse;
    vector<vector<bool>> _chip_inuse;

    vector<Router *> _routers;
    vector<ChipRouter *> _chip_routers;

    double _inner_chip_coef, _inter_chip_coef;

    enum{ buffer_empty=-1, buffer_exist, buffer_full };
    
    int _all_packets = 0;

    //simulation 끝난 후 결과를 알 수 있는 성분 넣어야 함. latency, Congestion, Area 등등.

public:
    Network(Configuration const * config, vector<vector<int>> & LUT);
    
    void CoutState();
    
    bool Virtual_Channel_Control(Router * router, Packet * pack);

    bool Inject(Router * router, Packet * pack, bool is_use, int dir, int src_x, int src_);
    bool Step(Router * router, ChipRouter * chiprouter, bool from_core, bool is_inject, int curbuffpos, int curpid, int dir, int cid, int rid, int src_x, int src_y, int dst_x, int dst_y);

    void Simulate();
    // void init_Networ();          //router, buffer 묶기
    // void Simulate();             //router안에 함수로 다른 router에 옮기거나 새로운 packet 생성
};

#endif