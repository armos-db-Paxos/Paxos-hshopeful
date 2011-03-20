/*
 * learner.hpp
 *
 *  Created on: Dec 3, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <iostream>
#include <string>
#include <set>

#include "player.hpp"
#include "protocol.hpp"

using namespace std;

namespace paxos {

/**
 * The role of Learner in Paxos algorithm
 */
template <typename ProposalT=string, typename MsgT=typename protocol<ProposalT>::message_type>
class learner : virtual public player<MsgT> {
 public:
	learner(int id, int port, const vector<pair<string, string> > &peers) : player<MsgT>(id, port, peers) { }
	virtual ~learner() { }
 protected:
	virtual void handle_request(const string &raw_message, boost::shared_ptr<udp::endpoint> remote_endpoint) {
		MsgT message = MsgT(protocol<ProposalT>::get_message_from_string(raw_message));
		if (protocol<ProposalT>::is_accept_response(message)) {
			cout << player<MsgT>::get_name() << " receives accept_response: " << message << endl;
			lock_.lock();
			if (proposal_done_.find(message.get_n()) == proposal_done_.end()) {
				execute_proposal(message.get_n(), message.get_proposal());
				proposal_done_.insert(message.get_n());
			}
			lock_.unlock();
		}
		else {
			// not able to handle the request, pass the information to the higher level
			throw true;
		}
	}
	virtual string get_player_type() const { return "learner"; }
	virtual void execute_proposal(int n, const ProposalT &proposal) const {
		// default behavior, just output
		cout << player<MsgT>::get_name() << " executes proposal[n=" << n << "]: " << proposal << endl;
	}
 private:
	set<int> proposal_done_; // record the proposals done
	boost::mutex lock_;
};


} // namespace paxos




