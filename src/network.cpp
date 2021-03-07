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
    _chip_buff_size = config->GetInt("chip_buff_size");

    _interchip_latency = config->GetInt("interchip_latency");

    _vir_chan = config->GetInt("vir_chan");
    _interval = config->GetInt("interval");

    _inner_chip_coef = config->GetFloat("inner_power_coef");
    _inter_chip_coef = config->GetFloat("inter_power_coef");

    _nodes = x_dim * y_dim * X_DIM * Y_DIM;
    _chips = X_DIM * Y_DIM;
    _nodesPerChip = x_dim * y_dim;
    
    // Allocate Initial Mapping Function
    FindIndexMap.resize(y_dim * Y_DIM);
    for (int i=0;i<y_dim * Y_DIM;i++) {
        FindIndexMap[i].resize(x_dim * X_DIM);
    }

    ChipFindIndexMap.resize(Y_DIM);
    for (int i=0;i<Y_DIM;i++) {
        ChipFindIndexMap[i].resize(X_DIM);
    }

    _parseip = config->GetStr("init_place");
    map<string, initPtr>::iterator ip_iter = InitFunction.find(_parseip);
    if (ip_iter == InitFunction.end()) {
        printf("Invalid Initial Placement function: %s ... ", _parseip.c_str());
        exit(-1);
    }
    ip = ip_iter->second;
    ip(phy_x, phy_y, phy_x_chip, phy_y_chip, _nodes, _nodesPerChip, x_dim, y_dim, X_DIM, Y_DIM, FindIndexMap, ChipFindIndexMap);

    // for (int j=0; j<y_dim*Y_DIM;j++){
    //     for (int i=0; i<x_dim*X_DIM; i++ ) {
    //         cerr << FindIndexMap[j][i] << " ";
    //     }
    //     cerr << endl;
    // }

    // for (int j=0; j<Y_DIM;j++){
    //     for (int i=0; i<X_DIM; i++ ) {
    //         cerr << ChipFindIndexMap[j][i] << " ";
    //     }
    //     cerr << endl;
    // }
    // exit(-1);

    // Allocate topology
    _topology = config->GetStr("topology");
    
    // Make Routers (CORES)
    _routers.resize(_nodes);
    for (int i=0;i<_nodes;i++) {
        vector<int> s_lut = vector<int> (LUT[i].begin(), LUT[i].end());
        _routers[i] = new Router(_topology, _buff_size, s_lut, normal, mul, i, phy_x, phy_y, phy_x_chip, phy_y_chip, ChipFindIndexMap);
        
        if ( _routers[i]->IsActive() ) {

            // printf("ROUTER NUM: %i, PACKETNUM: %i\n", i, _routers[i]->GetInjectNum());

            _all_packets += _routers[i]->GetInjectNum();
        }
    }

    // Make Routers (CHIPS)
    _chip_routers.resize(_chips);
    for (int i=0;i<_chips;i++) {
        _chip_routers[i] = new ChipRouter(_topology, _chip_buff_size, _interchip_latency);
    }
    

    // Make NET
    _net.resize(_nodes);
    for (int i=0;i<_nodes;i++) {
        _net[i].resize(_nodes);
    }

    _chip_net.resize(_chips);
    for (int i=0;i<_chips;i++) {
        _chip_net[i].resize(_chips);
    }
    
    // Make INUSE MATRIX
    _inuse.resize(_nodes);
    _num_buffer = _routers[0]->GetNumBuffer();
    for (int i=0;i<_nodes;i++) {
        _inuse[i].resize(_num_buffer);
    }

    _chip_inuse.resize(_chips);
    _num_buffer = _routers[0]->GetNumBuffer();
    for (int i=0;i<_chips;i++) {
        _chip_inuse[i].resize(_num_buffer);
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


bool Network::Virtual_Channel_Control(Router * router, Packet * pack)
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


bool Network::Step(Router * router, ChipRouter * chiprouter, bool from_core, bool is_inject, int curbuffpos, int curpid, int dir, int cid, int rid, int src_x, int src_y, int dst_x, int dst_y)
{
	bool is_use;

	bool TransPacket = false;

	int cuchipid = router->GetChipIdx();
	int ntrid = -1, ntchipid = -1;
	int ntbuffpos = -1;

	Router * ntrouter;
	ChipRouter * ntchiprouter;
    Packet * pack;
    
	if ( from_core ) {                  // CORE to CORE     or      CORE to CHIP
		if (dir == 0){							// GO LOCAL
			int BuffState = -1;

	        ntrid = FindIndexMap[src_y][src_x];
	        ntrouter = _routers[ntrid];

	        BuffState = ntrouter->GetLocalState();

	        if ( BuffState != 1) {
	            TransPacket = true;
	            ntbuffpos = localpos;

                if ( !is_inject ) {
                    pack = router->RemovePacket(curbuffpos, curpid);
                } else {
                    pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                    router->local_in_out++;
                }
                ntrouter->AddPacket(pack, ntbuffpos);
	        }
		} else {								// CORE2CORE, CORE2CHIP, CHIP2CORE
			bool BuffState = true;

			if (dir == 1) {
				ntrid = FindIndexMap[src_y][src_x - 1];
				ntrouter = _routers[ntrid];

				ntchipid = ntrouter->GetChipIdx();

				if (cuchipid == ntchipid) {

	    			BuffState = ntrouter->GetRightIsFull();
                    is_use = _inuse[rid][dir];
                    
	    			if( !BuffState && !is_use ) {
			            TransPacket = true;
			            ntbuffpos = rightpos;
                        _inuse[rid][dir] = true;
                        _net[rid][ntrid]++;

			            if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
			            ntrouter->AddPacket(pack, ntbuffpos);
			        }
				} else {
					ntchiprouter = _chip_routers[ntchipid];

					BuffState = ntchiprouter->GetRightIsFull();
                    is_use = _chip_inuse[cid][dir];
                    
					if( !BuffState && !is_use ) {
						TransPacket = true;
						ntbuffpos = rightpos;
                        _chip_inuse[cid][dir] = true;
                        _chip_net[cid][ntchipid]++;

						if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
						ntchiprouter->AddPacket(pack, ntbuffpos);
					}
				}
			} else if (dir == 2) {
				ntrid = FindIndexMap[src_y][src_x + 1];
		        ntrouter = _routers[ntrid];

		        ntchipid = ntrouter->GetChipIdx();

		        if (cuchipid == ntchipid) {
	    			BuffState = ntrouter->GetLeftIsFull();
                    is_use = _inuse[rid][dir];
                    
	    			if( !BuffState && !is_use ) {
			            TransPacket = true;
			            ntbuffpos = leftpos;
                        _inuse[rid][dir] = true;
                        _net[rid][ntrid]++;

			            if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
			            ntrouter->AddPacket(pack, ntbuffpos);
			        }
				} else {
					ntchiprouter = _chip_routers[ntchipid];
                    
                    // printf("CID: %i, RID: %i\n", cid, rid);

					BuffState = ntchiprouter->GetLeftIsFull();
                    is_use = _chip_inuse[cid][dir];
                    
					if( !BuffState && !is_use ) {
						TransPacket = true;
						ntbuffpos = leftpos;
                        _chip_inuse[cid][dir] = true;
                        _chip_net[cid][ntchipid]++;

						if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
						ntchiprouter->AddPacket(pack, ntbuffpos);
					}
				}
			} else if (dir == 3) {
				ntrid = FindIndexMap[src_y - 1][src_x];
		        ntrouter = _routers[ntrid];

		        ntchipid = ntrouter->GetChipIdx();

		        if (cuchipid == ntchipid) {
	    			BuffState = ntrouter->GetDownIsFull();
                    is_use = _inuse[rid][dir];

	    			if( !BuffState && !is_use ) {
			            TransPacket = true;
			            ntbuffpos = downpos;
                        _inuse[rid][dir] = true;
                        _net[rid][ntrid]++;

			            if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
			            ntrouter->AddPacket(pack, ntbuffpos);
			        }
				} else {
					ntchiprouter = _chip_routers[ntchipid];

					BuffState = ntchiprouter->GetDownIsFull();
                    is_use = _chip_inuse[cid][dir];

					if( !BuffState && !is_use ) {
						TransPacket = true;
						ntbuffpos = downpos;
                        _chip_inuse[cid][dir] = true;
                        _chip_net[cid][ntchipid]++;

						if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
						ntchiprouter->AddPacket(pack, ntbuffpos);
					}
				}
			} else if (dir == 4) {
				ntrid = FindIndexMap[src_y + 1][src_x];
		        ntrouter = _routers[ntrid];
		        
		        ntchipid = ntrouter->GetChipIdx();

		        if (cuchipid == ntchipid) {
	    			BuffState = ntrouter->GetUpIsFull();
                    is_use = _inuse[rid][dir];

	    			if( !BuffState && !is_use ) {
			            TransPacket = true;
			            ntbuffpos = uppos;
                        _inuse[rid][dir] = true;
                        _net[rid][ntrid]++;

			            if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
			            ntrouter->AddPacket(pack, ntbuffpos);
			        }
				} else {
					ntchiprouter = _chip_routers[ntchipid];

					BuffState = ntchiprouter->GetUpIsFull();
                    is_use = _chip_inuse[cid][dir];

					if( !BuffState && !is_use ) {
						TransPacket = true;
						ntbuffpos = uppos;
                        _chip_inuse[cid][dir] = true;
                        _chip_net[cid][ntchipid]++;

						if ( !is_inject ) {
                            pack = router->RemovePacket(curbuffpos, curpid);
                        } else {
                            pack = new Packet(false, dst_x, dst_y, rid);         // Get destination Info
                            router->InjectPacket();
                        }
						ntchiprouter->AddPacket(pack, ntbuffpos);
					}
				}
			}
		}
	} else {                    // CHIP to CORE
        bool BuffState = true;

        Packet * pack = chiprouter->GetPacketAddress(curbuffpos);
        
        rid = pack->last_node;
        src_x = phy_x[rid];
        src_y = phy_y[rid];

        is_use = _inuse[rid][dir];
        // cerr << "DIR: " << dir << endl;
		if (dir == 1) {
			ntrid = FindIndexMap[src_y][src_x - 1];
			ntrouter = _routers[ntrid];
            
			BuffState = ntrouter->GetRightIsFull();
            
			if( !BuffState && !is_use ) {
	            TransPacket = true;
	            ntbuffpos = rightpos;

                if (pack->cross_time < _interchip_latency) {
                    pack->cross_time++;
                } else {
                    chiprouter->RemovePacket(curbuffpos);
                    ntrouter->AddPacket(pack, ntbuffpos);
                    _inuse[rid][dir] = true;
                }
	        }
		} else if (dir == 2) {
			ntrid = FindIndexMap[src_y][src_x + 1];
			ntrouter = _routers[ntrid];

			BuffState = ntrouter->GetLeftIsFull();

			if( !BuffState && !is_use ) {
	            TransPacket = true;
	            ntbuffpos = leftpos;
                
                if (pack->cross_time < _interchip_latency) {
                    pack->cross_time++;
                } else {
                    chiprouter->RemovePacket(curbuffpos);
                    ntrouter->AddPacket(pack, ntbuffpos);
                    _inuse[rid][dir] = true;
                }
	        }
		} else if (dir == 3) {
			ntrid = FindIndexMap[src_y - 1][src_x];
			ntrouter = _routers[ntrid];

			BuffState = ntrouter->GetDownIsFull();

			if( !BuffState && !is_use ) {
	            TransPacket = true;
	            ntbuffpos = downpos;

                if (pack->cross_time < _interchip_latency) {
                    pack->cross_time++;
                } else {
                    chiprouter->RemovePacket(curbuffpos);
                    ntrouter->AddPacket(pack, ntbuffpos);
                    _inuse[rid][dir] = true;
                }
	        }
		} else if (dir == 4) {
			ntrid = FindIndexMap[src_y + 1][src_x];
			ntrouter = _routers[ntrid];

			BuffState = ntrouter->GetUpIsFull();

			if( !BuffState && !is_use ) {
	            TransPacket = true;
	            ntbuffpos = uppos;

                if (pack->cross_time < _interchip_latency) {
                    pack->cross_time++;
                } else {
                    chiprouter->RemovePacket(curbuffpos);
                    ntrouter->AddPacket(pack, ntbuffpos);
                    _inuse[rid][dir] = true;
                }
	        }
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

    while ( true )
    {
        total_cycle++;

        // printf(" ======= # ROUTING CYCLE: %i (PACKETS: %i) # =======\n", total_cycle, cur_inject_num);

        for_each(_inuse.begin(), _inuse.end(), [](vector<bool>& v){fill(v.begin(), v.end(), false);}); // Initialize <<_inuse>> Matrix
        for_each(_chip_inuse.begin(), _chip_inuse.end(), [](vector<bool>& v){fill(v.begin(), v.end(), false);}); // Initialize <<_inuse>> Matrix

        if ( total_cycle % 500 == 0 ) { printf("Running ... ( CYCLES: %i \t PACKETS RECIEVED : %i / %i )\n", total_cycle, cur_inject_num, _all_packets); }

        bool busy = false;

        Router * router;
        ChipRouter * chiprouter;

        // INTER CHIP
        for ( int cid = 0; cid < _chips; cid++ )
        {
            int src_x, src_y;

            chiprouter = _chip_routers[cid];

            // CHIP_LEFT
            if ( !chiprouter->GetLeftIsEmpty() ) {
                Packet * pack = chiprouter->GetPacketAddress(leftpos);
                
                int temprid = pack->last_node;
                src_x = phy_x[temprid];
                src_y = phy_y[temprid];

                if ( !pack->is_move ) {
                    int dst_x = pack->dst_x, dst_y = pack->dst_y;
                    int dir = rf(src_x, src_y, dst_x, dst_y);

                    bool Q_busy = Step(router, chiprouter, false, false, leftpos, 0, dir, cid, 0, 0, 0, 0, 0);

                    if ( Q_busy ) {
                        busy = true;
                    }
                }
            }

            // CHIP_RIGHT
            if ( !chiprouter->GetRightIsEmpty() ) {
                Packet * pack = chiprouter->GetPacketAddress(rightpos);

                int temprid = pack->last_node;
                src_x = phy_x[temprid];
                src_y = phy_y[temprid];

                if ( !pack->is_move ) {
                    int dst_x = pack->dst_x, dst_y = pack->dst_y;
                    int dir = rf(src_x, src_y, dst_x, dst_y);

                    // cerr << "DIR: " << dir << ", (SRCX, SRCY): " << src_x << ", " << src_y << ", (DSTX, DSTY): " << dst_x << ", " << dst_y << endl;

                    bool Q_busy = Step(router, chiprouter, false, false, rightpos, 0, dir, cid, 0, 0, 0, 0, 0);

                    if ( Q_busy ) {
                        busy = true;
                    }
                }
            }
            
            // CHIP_UP
            if ( !chiprouter->GetUpIsEmpty() ) {
                Packet * pack = chiprouter->GetPacketAddress(uppos);

                int temprid = pack->last_node;
                src_x = phy_x[temprid];
                src_y = phy_y[temprid];

                if ( !pack->is_move ) {
                    int dst_x = pack->dst_x, dst_y = pack->dst_y;
                    int dir = rf(src_x, src_y, dst_x, dst_y);

                    bool Q_busy = Step(router, chiprouter, false, false, uppos, 0, dir, cid, 0, 0, 0, 0, 0);

                    if ( Q_busy ) {
                        busy = true;
                    }
                }
            }

            // CHIP_DOWN
            if ( !chiprouter->GetDownIsEmpty() ) {
                Packet * pack = chiprouter->GetPacketAddress(downpos);

                int temprid = pack->last_node;
                src_x = phy_x[temprid];
                src_y = phy_y[temprid];

                if ( !pack->is_move ) {
                    int dst_x = pack->dst_x, dst_y = pack->dst_y;
                    int dir = rf(src_x, src_y, dst_x, dst_y);

                    bool Q_busy = Step(router, chiprouter, false, false, downpos, 0, dir, cid, 0, 0, 0, 0, 0);

                    if ( Q_busy ) {
                        busy = true;
                    }
                }
            }
        }
        
        // INNER CHIP
        for ( int rid = 0; rid < _nodes; rid++ ) 
        {   
            int src_x = phy_x[rid];
            int src_y = phy_y[rid];

            router = _routers[rid];

            int ChipIdx = router->GetChipIdx();
            chiprouter = _chip_routers[ChipIdx];
    
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
                            
                            bool Q_busy = Step(router, chiprouter, true, false, localpos, pid, dir, 0, rid, src_x, src_y, 0, 0);

                            if ( Q_busy ) {
                                busy = true;
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

                int dir = rf(src_x, src_y, dst_x, dst_y);
                
                // printf("RID: %i, DST: %i, DIR: %i, CID: %i\n", rid, dst, dir, router->GetChipIdx());
                bool Q_busy = Step(router, chiprouter, true, true, 0, 0, dir, router->GetChipIdx(), rid, src_x, src_y, dst_x, dst_y);

                if ( Q_busy ) {
                    busy = true;
                }
            }
            
            // LEFT_CHANNEL
            if ( !router->GetLeftIsEmpty() ) {
                for ( int pid=0; pid<router->GetNum(leftpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(leftpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channel_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);

                        bool Q_busy = Step(router, chiprouter, true, false, leftpos, pid, dir, router->GetChipIdx(), rid, src_x, src_y, 0, 0);

                        if ( Q_busy ) {
                            busy = true;
                        }
                    }
                }
            }
            
            // RIGTH_CHANNEL
            if ( !router->GetRightIsEmpty() ) {
                for ( int pid=0; pid<router->GetNum(rightpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(rightpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channel_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        
                        bool Q_busy = Step(router, chiprouter, true, false, rightpos, pid, dir, router->GetChipIdx(), rid, src_x, src_y, 0, 0);

                        if ( Q_busy ) {
                            busy = true;
                        }
                    }
                }
            }
            
            // UP_CHANNEL
            if ( !router->GetUpIsEmpty() ) {
                for ( int pid=0; pid<router->GetNum(uppos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(uppos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channel_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        
                        bool Q_busy = Step(router, chiprouter, true, false, uppos, pid, dir, router->GetChipIdx(), rid, src_x, src_y, 0, 0);

                        if ( Q_busy ) {
                            busy = true;
                        }
                    }
                }
            }
            
            // DOWN_CHANNEL
            if ( !router->GetDownIsEmpty() ) {
                for ( int pid=0; pid<router->GetNum(downpos); pid++ ) {
                    Packet * pack = router->GetPacketAddress(downpos, pid);
                    if ( !pack->is_move ) {
                        bool _vir_able = Virtual_Channel_Control(router, pack);

                        if ( _vir_able ) {
                            busy = true;
                            continue;
                        }

                        int dst_x = pack->dst_x, dst_y = pack->dst_y;
                        int dir = rf(src_x, src_y, dst_x, dst_y);
                        
                        bool Q_busy = Step(router, chiprouter, true, false, downpos, pid, dir, router->GetChipIdx(), rid, src_x, src_y, 0, 0);

                        if ( Q_busy ) {
                            busy = true;
                        }
                    }
                }
            }
        }
        
        if ( cur_inject_num == _all_packets ) {
            printf("Routing done ... (cycles: %i) \n", total_cycle);

            // Calculate Power Consumption
            cerr << endl<< setw(50) << setfill('#') << " " << endl;
            cerr << "=> SIMULATION RESULT !! " << endl;
            cerr << setw(50) << setfill('#') << " " << endl;

            printf("\nLatency (cycles) : %i\n", total_cycle);
            double inner_comm = 0.;
            int inner_cong = 0;
            for (int i=0; i<(int)_net.size(); i++) {
                for (int j=0; j<(int)_net[i].size(); j++) {
                    inner_comm += (double)_net[i][j];
                    if (inner_cong < _net[i][j]) {
                        inner_cong = _net[i][j];
                    }
                }
            }
            
            double inter_comm = 0.;
            int inter_cong = 0;
            for (int i=0; i<(int)_chip_net.size(); i++) {
                for (int j=0; j<(int)_chip_net[i].size(); j++) {
                    inter_comm += (double)_chip_net[i][j];
                    if (inter_cong < _chip_net[i][j]) {
                        inter_cong = _chip_net[i][j];
                    }
                }
            }
            printf("Power Consumption : %.2f\n", inner_comm * _inner_chip_coef + inter_comm * _inter_chip_coef);

            printf("\nCommnication Cost (Inner-Chip) : %.2f\n", inner_comm);
            printf("Commnication Cost (Inter-Chip) : %.2f\n", inter_comm);

            printf("\nMax Bandwidth Req (Inner-Chip) : %i\n", inner_cong);
            printf("Max Bandwidth Req (Inter-Chip) : %i\n", inter_cong);

            printf("\n");

            break;
        } else if ( cur_inject_num < _all_packets && !busy ) {
            printf("Deadlock ... Aborting routing ... (cycles: %i)\n", total_cycle);
            break;
        }

        for (int i=0;i<_nodes;i++) {
            _routers[i]->Reset();
        }

        for (int i=0;i<_chips;i++) {
            _chip_routers[i]->Reset();
        }

        // cerr << _routers[1]->GetLeftIsEmpty() <<endl;
        // exit(-1);
    }
}