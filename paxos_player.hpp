/*
 * paxos_player.hpp
 *
 *  Created on: Dec 3, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include "proposer.hpp"
#include "acceptor.hpp"
#include "learner.hpp"
#include "protocol.hpp"

using namespace std;

namespace paxos {

/**
 * The combined role of Proposer, Acceptor, and Learner in Paxos algorithm
 */
template <typename ProposalT=string, typename MsgT=typename protocol<ProposalT>::message_type>
class paxos_player : public proposer<ProposalT, MsgT>, public acceptor<ProposalT, MsgT>, public learner<ProposalT, MsgT> {
 public:
	paxos_player(int id, int port, const vector<pair<string, string> > &peers) :
		player<MsgT>(id, port, peers),
		proposer<ProposalT, MsgT>(id, port, peers),
		acceptor<ProposalT, MsgT>(id, port, peers),
		learner<ProposalT, MsgT>(id, port, peers) { }
	virtual ~paxos_player() { }
 protected:
	virtual void handle_request(const string &raw_message, boost::shared_ptr<udp::endpoint> remote_endpoint) {
		// handle request by super classes
		try {
			learner<ProposalT, MsgT>::handle_request(raw_message, remote_endpoint);
		} catch (bool bit) {
			try {
				proposer<ProposalT, MsgT>::handle_request(raw_message, remote_endpoint);
			} catch (bool bit) {
				try {
					acceptor<ProposalT, MsgT>::handle_request(raw_message, remote_endpoint);
				} catch (...) {
					cerr << player<MsgT>::get_name() << " cannot handle request(raw message): " << raw_message << endl;
				}
			}
		}
	}
	virtual string get_player_type() const { return "paxos_player"; }
};


} // namespace paxos




