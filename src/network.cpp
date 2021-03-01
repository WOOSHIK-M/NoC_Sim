#include <typeinfo>
#include <iomanip>
#include <map>
#include <algorithm>
#include <cstring>

#include "network.hpp"


Network::Network(Configuration const * config, vector<vector<int>> & LUT) 
{   
    cerr << endl<< setw(50) << setfill('#') << " " << endl;
    cerr << "=> Make New network !" << endl;
    cerr << setw(50) << setfill('#') << " " << endl;

    // Multicast Parameters
    normal = config->GetInt("lut_normal");
    mul = config->GetInt("lut_mul");

    // Build Basic Network
    x_dim = config->GetInt("x_dim");
    y_dim = config->GetInt("y_dim");
    X_DIM = config->GetInt("X_DIM");
    Y_DIM = config->GetInt("Y_DIM");
    _buff_size = config->GetInt("buff_size");

    _vir_chan = config->GetInt("vir_chan");
    _interval = config->GetInt("interval");

    _nodes = x_dim * y_dim * X_DIM * Y_DIM;
    
    // Allocate Initial Mapping Function
    FindIndexMap.resize(y_dim * Y_DIM);
    for (int i=0;i<y_dim * Y_DIM;i++) {
        FindIndexMap[i].resize(x_dim * X_DIM);
    }

    _parseip = config->GetStr("init_place");
    map<string, initPtr>::iterator ip_iter = InitFunction.find(_parseip);
    if (ip_iter == InitFunction.end()) {
        printf("Invalid Initial Placement function: %s ... ", _parseip.c_str());
        exit(-1);
    }
    ip = ip_iter->second;
    ip(phy_x, phy_y, _nodes, x_dim, y_dim, X_DIM, Y_DIM, FindIndexMap);
    
    // for (int i=0; i<(int)phy_x.size(); i++){
    //     cerr << phy_x[i] << " ";
    // }
    // cerr << endl;
    // for (int i=0; i<(int)phy_x.size(); i++){
    //     cerr << phy_y[i] << " ";
    // }
    // cerr << endl;
    // exit(-1);

    // Allocate topology
    _topology = config->GetStr("topology");
    
    // Make Routers
    _routers.resize(_nodes);
    for (int i=0;i<_nodes;i++) {
        vector<int> s_lut = vector<int> (LUT[i].begin(), LUT[i].end());
        _routers[i] = new Router( _topology, _buff_size, s_lut, normal, mul, i, phy_x, phy_y);
        
        if ( _routers[i]->IsActive() ) {

            printf("ROUTER NUM: %i, PACKETNUM: %i\n", i, _routers[i]->GetInjectNum());

            _all_packets += _routers[i]->GetInjectNum();
        }
    }
        
    _net.resize(_nodes);
    for (int i=0;i<_nodes;i++) {
        _net[i].resize(_nodes);
    }
    
    _inuse.resize(_nodes);
    _num_buffer = _routers[0]->GetNumBuffer();
    for (int i=0;i<_nodes;i++) {
        _inuse[i].resize(_num_buffer);
    }

    // Allocate Routing Function
    _parserf = config->GetStr("routing_function");
    map<string, routingPtr>::iterator rf_iter = routingFunction.find( _parserf );
    if (rf_iter == routingFunction.end()) {
        printf("Invalid routing function: %s ... ", _parserf.c_str());
        exit(-1);
    }
    rf = rf_iter->second;
}


void Network::CoutState()
{
    printf("\nRouting Function: %s \n", _parserf.c_str());
    printf("\nThe number of Chips : %i\nThe number of Cores : %i\n", (int) X_DIM * Y_DIM, (int)x_dim * y_dim);
    printf("\nThis Network has %i routers ... \n", (int)_routers.size());
    _routers[0]->CoutState();
}


bool Network::Virtual_Channle_Control(Router * router, Packet * pack)
{
    bool Able = false;

    if (pack->buff_pos == 0 && router->_time > _interval) {
        Able = true;
    } else if (pack->buff_pos == -1 && router->local_in_out == 1) {
        Able = true;
    } else if (pack->buff_pos == 1 && router->GetNumOut(leftpos) == _vir_chan) {
        Able = true;
    } else if (pack->buff_pos == 2 && router->GetNumOut(rightpos) == _vir_chan) {
        Able = true;
    } else if (pack->buff_pos == 3 && router->GetNumOut(uppos) == _vir_chan) {
        Able = true;
    } else if (pack->buff_pos == 4 && router->GetNumOut(downpos) == _vir_chan) {
        Able = true;
    } 

    return Able;
}


bool Network::Inject(Router * router, Packet * pack, bool is_use, int dir, int src_x, int src_y)
{
    bool TransPacket = false;

    int ntrid = -1;
    int ntbuffpos = -1;

    Router * ntrouter;

    if (dir == 0) {
        int BuffState = -1;

        ntrid = FindIndexMap[src_y][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetLocalState();
        
        if ( BuffState != 1 ) {
            TransPacket = true;
            ntbuffpos = localpos;
        }
    } else if (dir == 1 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y][src_x - 1];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetRightIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = rightpos;
        }
    } else if (dir == 2 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y][src_x + 1];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetLeftIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = leftpos;
        }
    } else if (dir == 3 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y - 1][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetDownIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = downpos;
        }
    } else if (dir == 4 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y + 1][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetUpIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = uppos;
        }
    }

    if (TransPacket) {
        router->InjectPacket();
        ntrouter->AddPacket(pack, ntbuffpos);
        router->local_in_out++;
    }

    return TransPacket;
}


bool Network::Step(Router * router, bool is_use, int curbuffpos, int curpid, int dir, int src_x, int src_y)
{
    bool TransPacket = false;
    
    int ntrid = -1;
    int ntbuffpos = -1;

    Router * ntrouter;

    if (dir == 0) {
        int BuffState = -1;

        ntrid = FindIndexMap[src_y][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetLocalState();
        
        if ( BuffState != 1 ) {
            TransPacket = true;
            ntbuffpos = localpos;

            Packet * pack = router->RemovePacket(curbuffpos, curpid);
            ntrouter->AddPacket(pack, ntbuffpos);
        }
    } else if (dir == 1 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y][src_x - 1];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetRightIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = rightpos;

            Packet * pack = router->RemovePacket(curbuffpos, curpid);
            ntrouter->AddPacket(pack, ntbuffpos);
        }
    } else if (dir == 2 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y][src_x + 1];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetLeftIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = leftpos;

            Packet * pack = router->RemovePacket(curbuffpos, curpid);
            ntrouter->AddPacket(pack, ntbuffpos);
        }
    } else if (dir == 3 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y - 1][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetDownIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = downpos;

            Packet * pack = router->RemovePacket(curbuffpos, curpid);
            ntrouter->AddPacket(pack, ntbuffpos);
        }
    } else if (dir == 4 && !is_use) {
        bool BuffState = true;

        ntrid = FindIndexMap[src_y + 1][src_x];
        ntrouter = _routers[ntrid];

        BuffState = ntrouter->GetUpIsFull();

        if( !BuffState ) {
            TransPacket = true;
            ntbuffpos = uppos;

            Packet * pack = router->RemovePacket(curbuffpos, curpid);
            ntrouter->AddPacket(pack, ntbuffpos);
        }
    }

    return TransPacket;
}


void Network::Simulate()
{
    cerr << endl<< setw(50) << setfill('#') << " " << endl;
    cerr << "=> Start Simulation !! " << endl;
    cerr << setw(50) << setfill('#') << " " << endl;

    int total_cycle = 0;

    bool cur_state = true;

    int cur_inject_num = 0;
    
    printf("ALLPACKETS: %i\n", _all_packets);
    // exit(-1);

    while ( true )
    {
        total_cycle++;

        // printf(" ======= # ROUTING CYCLE: %i (PACKETS: %i) # =======\n", total_cycle, cur_inject_num);

        for_each(_inuse.begin(), _inuse.end(), [](vector<bool>& v){fill(v.begin(), v.end(), false);}); // Initialize <<_inuse>> Matrix

        if ( total_cycle % 500 == 0 ) { printf("Running ... ( %i / %i )\n", cur_inject_num, _all_packets); }

        bool busy = false;

        for ( int rid = 0; rid < _nodes; rid++ ) 
        {   
            int src_x = phy_x[rid];
            int src_y = phy_y[rid];

            Router * router = _routers[rid];
            
            // LOCAL_OUT_CHANNEL
            cur_state = router->GetLocalIsEmpty();
            if ( !cur_state ) {
                for ( int pid=0; pid<router->GetNum(localpos); pid++ ) {              // local_num 추가
                    Packet * pack = router->GetPacketAddress(localpos, pid);
                    if ( router->IsMul() ) {
                        if ( !pack->is_move ) {
                            router->RenewPacket(pack);                  // convert to multicast packet
                                            
                            int dst_x = pack->dst_x, dst_y = pack->dst_y;
                            int dir = rf(src_x, src_y, dst_x, dst_y);       // find direction depending on routing function

                            bool Q_busy = Step(router, _inuse[rid][dir], localpos, pid, dir, src_x, src_y);

                            if ( Q_busy ) {
                                busy = true;
                                _inuse[rid][dir]= true;
                            }
                        }
                    } else {
                        // printf("PACKINFO: (move) = (%i) .. (sx, sy) = (%i, %i) .. (dx, dy) = (%i, %i) .. DIRECTION: %i\n", (int)pack->is_move, src_x, src_y, pack->dst_x, pack->dst_y, 0);
                        router->ArrivedPacket(localpos, pid);
                        cur_inject_num++;
                    }
                }
            }

            // LOCAL_IN_CHANNEL
            if ( !router->GetInjectIsEmpty() ) {
                int dst = router->GetNewDest();
                int dst_x = phy_x[dst];
                int dst_y = phy_y[dst];
            
                Packet * pack = new Packet(false, dst_x, dst_y);
                bool _vir_able = Virtual_Channle_Control(router, pack);

                if ( _vir_able ) {
                    busy = true;
                    continue;
                }

                int dir = rf(src_x, src_y, dst_x, dst_y);
                bool Q_busy = Inject(router, pack, _inuse[rid][dir], dir, src_x, src_y);

                if ( Q_busy ) {
                    busy = true;
                    _inuse[rid][dir] = true;
                    // printf("NUM: %i, INJECT ... ", rid);
                    // printf("PACKINFO: (move) = (%i), (sx, sy) = (%i, %i) .. (dx, dy) = (%i, %i) .. DIRECTION: %i\n", (int)pack->is_move, src_x, src_y, pack->dst_x, pack->dst_y, dir);
                }
            }

            // LEFT_CHANNEL
            cur_state = router->GetLeftIsEmpty();
            if ( !cur_state ) {
                for ( int pid=0; pid<router->GetNum(leftpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(leftpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channle_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);

                        // printf("PACKINFO: (move) = (%i), (sx, sy) = (%i, %i) .. (dx, dy) = (%i, %i) .. DIRECTION: %i\n", (int)pack->is_move, src_x, src_y, pack->dst_x, pack->dst_y, dir);

                        bool Q_busy = Step(router, _inuse[rid][dir], leftpos, pid, dir, src_x, src_y);

                        if ( Q_busy ) {
                            busy = true;
                            _inuse[rid][dir]= true;
                        }
                    }
                }
            }

            // RIGTH_CHANNEL
            cur_state = router->GetRightIsEmpty();
            if ( !cur_state ) {
                for ( int pid=0; pid<router->GetNum(rightpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(rightpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channle_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        bool Q_busy = Step(router, _inuse[rid][dir], rightpos, pid, dir, src_x, src_y);

                        if ( Q_busy ) {
                            busy = true;
                            _inuse[rid][dir]= true;
                        }
                    }
                }
            }
            
            // UP_CHANNEL
            cur_state = router->GetUpIsEmpty();
            if ( !cur_state ) {
                for ( int pid=0; pid<router->GetNum(uppos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(uppos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channle_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        bool Q_busy = Step(router, _inuse[rid][dir], uppos, pid, dir, src_x, src_y);

                        if ( Q_busy ) {
                            busy = true;
                            _inuse[rid][dir]= true;
                        }
                    }
                }
            }
            
            // DOWN_CHANNEL
            cur_state = router->GetDownIsEmpty();
            if ( !cur_state ) {
                for ( int pid=0; pid<router->GetNum(downpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(downpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channle_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        bool Q_busy = Step(router, _inuse[rid][dir], downpos, pid, dir, src_x, src_y);

                        if ( Q_busy ) {
                            busy = true;
                            _inuse[rid][dir]= true;
                        }
                    }
                }
            }
        }

        if ( cur_inject_num == _all_packets ) {
            printf("Routing done ... (cycles: %i) \n", total_cycle);
            break;
        } else if ( cur_inject_num < _all_packets && !busy ) {
            printf("Deadlock ... Aborting routing ... \n");
            break;
        }

        for (int i=0;i<_nodes;i++) {
            _routers[i]->Reset();
        }

        // cerr << _routers[1]->GetLeftIsEmpty() <<endl;
        // exit(-1);
    }
}