/*
 * player_factory.hpp
 *
 *  Created on: Dec 4, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "player.hpp"
#include "protocol.hpp"
#include "paxos_player.hpp"

using namespace std;

namespace paxos {

/**
 * Player_factory creates player objects of different roles
 */
template <typename ProposalT=string, typename MsgT=typename protocol<ProposalT>::message_type>
class player_factory {
 public:
	static boost::shared_ptr<player<MsgT> > get_player(const string &type, int id, int mainport, const vector<pair<string, string> > &peers) {
		player<MsgT> *p;
		if (type == "paxos_player") p = new paxos_player<ProposalT, MsgT>(id, mainport, peers);
		else if (type == "proposer") p = new proposer<ProposalT, MsgT>(id, mainport, peers);
		else if (type == "acceptor") p = new acceptor<ProposalT, MsgT>(id, mainport, peers);
		else if (type == "learner") p = new learner<ProposalT, MsgT>(id, mainport, peers);
		else {
			cerr << "Wrong type in player_factory::get_player(): " << type << endl;
			exit(1);
		}

		return boost::shared_ptr<player<MsgT> >(p);
	}
};


}



