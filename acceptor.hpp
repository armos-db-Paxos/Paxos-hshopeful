/*
 * acceptor.hpp
 *
 *  Created on: Dec 3, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "player.hpp"
#include "protocol.hpp"

using namespace std;

namespace paxos {

/**
 * The role of Acceptor in Paxos algorithm
 */
template <typename ProposalT=string, typename MsgT=typename protocol<ProposalT>::message_type>
class acceptor : virtual public player<MsgT> {
 public:
	acceptor(int id, int port, const vector<pair<string, string> > &peers) : player<MsgT>(id, port, peers) {
		// read state
		ifstream ifs(get_state_file_name());
		if (!ifs) {
			// no state file, create one
			ofstream ofs(get_state_file_name());
			ofs << -1 << endl
				<< -1 << endl
				<< ProposalT() << endl;
			ofs.close();
			// initialize state
			highest_prepare_request_number_responded_ = -1;
			highest_accepted_proposal_.first = -1;
			highest_accepted_proposal_.second = ProposalT();
		}
		else {
			ifs >> highest_prepare_request_number_responded_ >> highest_accepted_proposal_.first >> highest_accepted_proposal_.second;
			ifs.close();
		}
	}
	virtual ~acceptor() {
		save_state();
	}
 protected:
	virtual void handle_request(const string &raw_message, boost::shared_ptr<udp::endpoint> remote_endpoint) {
		MsgT message = MsgT(protocol<ProposalT>::get_message_from_string(raw_message));
		if (protocol<ProposalT>::is_prepare_request(message)) {
			cout << player<MsgT>::get_name() << " receives prepare_request: " << message << endl;
			if (!(highest_prepare_request_number_responded_ > message.get_n()) &&
				!(highest_accepted_proposal_.first == message.get_n())) {
				// return prepare_response
				typename protocol<ProposalT>::message_type prepare_response =
						protocol<ProposalT>::get_prepare_response(message.get_n(), highest_accepted_proposal_.first, highest_accepted_proposal_.second);
				send_message_back(prepare_response, remote_endpoint);
				cout << player<MsgT>::get_name() << " sends prepare_response back: " << prepare_response << endl;
				// update and save state
				update_mutex_.lock();
				highest_prepare_request_number_responded_ = message.get_n();
				save_state();
				update_mutex_.unlock();
			}
		}
		else if (protocol<ProposalT>::is_accept_request(message)) {
			cout << player<MsgT>::get_name() << " receives accept_request: " << message << endl;
			if (!(highest_prepare_request_number_responded_ > message.get_n())) {
				// return accept_response
				typename protocol<ProposalT>::message_type accept_response =
						protocol<ProposalT>::get_accept_response(message.get_n(), message.get_proposal());
				send_message_to_all(accept_response);
				cout << player<MsgT>::get_name() << " sends accept_response to all: " << accept_response << endl;
				// update and save state
				update_mutex_.lock();
				if (message.get_n() > highest_accepted_proposal_.first) {
					highest_accepted_proposal_.first = message.get_n();
					highest_accepted_proposal_.second = message.get_proposal();
				}
				save_state();
				update_mutex_.unlock();
			}
		}
		else {
			// not able to handle the request, pass the information to the higher level
			throw true;
		}
	}
	virtual string get_player_type() const { return "acceptor"; }
 private:
	const char *get_state_file_name() const { return (player<MsgT>::get_id_string() + "_state.txt").c_str(); }
	void save_state() const {
		// write state to file
		ofstream ofs(get_state_file_name());
		ofs << highest_prepare_request_number_responded_ << endl
			<< highest_accepted_proposal_.first << endl
			<< highest_accepted_proposal_.second << endl;
		ofs.close();
	}
	boost::mutex update_mutex_;
	int highest_prepare_request_number_responded_;
	pair<int, ProposalT> highest_accepted_proposal_;
};


} // namespace paxos
