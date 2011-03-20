/*
 * protocol.hpp
 *
 *  Created on: Dec 2, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>
#include <sstream>

using namespace std;

namespace paxos {

/**
 * Protocol defines the protocol used in the Paxos implementation
 */
template <typename ProposalT=string>
class protocol {
	// all methods are static
 public:
	// for proposer, to get the next number for the proposal to be proposed
	static int get_number(int id, int current_number = -1) {
		if (current_number == -1) { // initial situation
			return id;
		}
		// counter + id. Ex. for id = 7, the sequence is: 7, 17, 27, ..., 107, 117
		int counter = current_number / 10;
		return ++counter * 10 + id;
	}

	// unified message type
	typedef class message {
		friend class protocol<ProposalT>;
	 public:
		operator string() const {
			ostringstream oss;
			oss << type_ << " ";
			if (is_client_request()) {
				oss << message_content_ << " ";
				return oss.str();
			}
			oss << n_ << " ";
			if (is_prepare_request()) {

			} else if (is_prepare_response()) {
				oss << previous_n_ << " ";
				if (previous_n_ != -1) oss << proposal_ << " ";
			} else if (is_accept_request() || is_accept_response()) {
				oss << proposal_ << " ";
			} else {
				cerr << "Wrong message type in protocol::operator string()" << endl;
			}
			oss << message_content_;
			return oss.str();
		}

		int get_n() const { return n_; }
		int get_previous_n() const { return previous_n_; }
		ProposalT get_proposal() const { return proposal_; }
		string get_message_content() const { return message_content_; }

		friend ostream &operator<< (ostream &os, const message &msg) {
			os << string(msg);
			return os;
		}
	 private:
		bool is_client_request() const { return type_ == client_request; }
		bool is_prepare_request() const { return type_ == prepare_request; }
		bool is_prepare_response() const { return type_ == prepare_response; }
		bool is_accept_request() const { return type_ == accept_request; }
		bool is_accept_response() const { return type_ == accept_response; }
		enum type { client_request, prepare_request, prepare_response, accept_request, accept_response } type_;
		int n_;
	    int previous_n_;
		ProposalT proposal_;
		string message_content_; // this may not exist at all
	} message_type;

	// creating messages
	static message_type get_message_from_string(const string &str) {
		message result;
		istringstream iss(str);
		int type;
		iss >> type;
		result.type_ = static_cast<typename message::type>(type);
		if (result.is_client_request()) {

		}
		else if (result.is_prepare_request()) {
			iss >> result.n_;
		} else if (result.is_prepare_response()) {
			iss >> result.n_;
			iss >> result.previous_n_;
			if (result.previous_n_ != -1) iss >> result.proposal_;
		} else if (result.is_accept_request() || result.is_accept_response()) {
			iss >> result.n_;
			iss >> result.proposal_;
		} else {
			cerr << "Wrong message type in protocol::get_message_from_string(): " << str << endl;
		}
		getline(iss, result.message_content_);
		return result;
	}

	static message_type get_client_request(const string &content) {
		message result;
		result.type_ = message::client_request;
		result.message_content_ = content;
		return result;
	}

	static message_type get_prepare_request(int n) {
		message result;
		result.type_ = message::prepare_request;
		result.n_ = n;
		return result;
	}

	static message_type get_prepare_response(int n, int previous_n, const ProposalT &proposal) {
		message result;
		result.type_ = message::prepare_response;
		result.n_ = n;
		result.previous_n_ = previous_n;
		result.proposal_ = proposal;
		return result;
	}

	static message_type get_accept_request(int n, const ProposalT &proposal) {
		message result;
		result.type_ = message::accept_request;
		result.n_ = n;
		result.proposal_ = proposal;
		return result;
	}

	static message_type get_accept_response(int n, const ProposalT &proposal) {
		message result;
		result.type_ = message::accept_response;
		result.n_ = n;
		result.proposal_ = proposal;
		return result;
	}

	// test message type
	static bool is_client_request(const message_type &msg) { return msg.is_client_request(); }
	static bool is_prepare_request(const message_type &msg) { return msg.is_prepare_request(); }
	static bool is_prepare_response(const message_type &msg) { return msg.is_prepare_response(); }
	static bool is_accept_request(const message_type &msg) { return msg.is_accept_request(); }
	static bool is_accept_response(const message_type &msg) { return msg.is_accept_response(); }
 private:
	protocol() { }
	protocol(const protocol &) {}
};


} // namespace Paxos




