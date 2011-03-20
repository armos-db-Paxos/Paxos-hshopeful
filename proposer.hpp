/*
 * proposer.hpp
 *
 *  Created on: Dec 2, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <iostream>
#include <map>
#include <string>

#include "player.hpp"
#include "protocol.hpp"

using namespace std;

namespace paxos {

/**
 * The role of Proposer in Paxos algorithm
 */
template <typename ProposalT=string, typename MsgT=typename protocol<ProposalT>::message_type>
class proposer : virtual public player<MsgT> {
 public:
	proposer(int id, int port, const vector<pair<string, string> > &peers) :
		player<MsgT>(id, port, peers), current_number_(protocol<ProposalT>::get_number(id)), peer_counter_(peers.size()) { }
	virtual ~proposer() { }
 protected:
	virtual void handle_request(const string &raw_message, boost::shared_ptr<udp::endpoint> remote_endpoint) {
		MsgT message = MsgT(protocol<ProposalT>::get_message_from_string(raw_message));
		if (protocol<ProposalT>::is_client_request(message)) {
			cout << player<MsgT>::get_name() << " receives client_request: " << message << endl;
			update_mutex_.lock();
			int n = get_and_update_current_number();
			accept_counter_[n] = make_pair(0, make_pair(-1, ProposalT()));
			update_mutex_.unlock();
			typename protocol<ProposalT>::message_type prepare_request = protocol<ProposalT>::get_prepare_request(n);
			send_message_to_all(prepare_request);
			cout << player<MsgT>::get_name() << " sends prepare_request to all: " << prepare_request << endl;
		}
		else if (protocol<ProposalT>::is_prepare_response(message)) {
			cout << player<MsgT>::get_name() << " receives prepare_response: " << message << endl;
			int n = message.get_n();
			int cnt = -1;
			update_mutex_.lock();
			if (accept_counter_.find(n) != accept_counter_.end()) {
				cnt = ++accept_counter_[n].first;
				int pre_n = message.get_previous_n();
				if (pre_n != -1) {
					pair<int, ProposalT> &pp = accept_counter_[n].second;
					if (pre_n > pp.first) {
						pp.second = message.get_proposal();
					}
				}
			}
			// test if the number of responses just reaches the majority
			if (has_just_reached_majority(cnt)) {
				typename protocol<ProposalT>::message_type accept_request =
						protocol<ProposalT>::get_accept_request(n,
								accept_counter_[n].second.first == -1 ? ProposalT(get_proposal(n)) :
																		accept_counter_[n].second.second);
				send_message_to_all(accept_request);
				cout << player<MsgT>::get_name() << " sends accept_request to all: " << accept_request << endl;
			}
			update_mutex_.unlock();
		}
		else if (protocol<ProposalT>::is_accept_response(message)) {
			cout << player<MsgT>::get_name() << " receives accept_response: " << message << endl;
		}
		else {
			// not able to handle the request, pass the information to the higher level
			throw true;
		}
	}
	virtual string get_player_type() const { return "proposer"; }
 private:
	// no space should exist in the proposal string
	string get_proposal(int n) const {
		ostringstream oss;
		oss << "Proposal_by_proposer[id=" << player<MsgT>::id_ << "]_for_[n=" << n << "]";
		return oss.str();
	}
	bool has_just_reached_majority(int cnt) const {
		return cnt == ceil(peer_counter_ / 2.0);
	}
	int get_current_number() const { return current_number_; }
	boost::mutex update_mutex_;
	int get_and_update_current_number() {
		int tmp = current_number_;
		current_number_ = protocol<ProposalT>::get_number(player<MsgT>::id_, current_number_);
		return tmp;
	}
	int current_number_; // the current number of the next proposal to be proposed
	int peer_counter_;
	map<int, pair<size_t, pair<int, ProposalT> > > accept_counter_;
};


} // namespace Paxos






