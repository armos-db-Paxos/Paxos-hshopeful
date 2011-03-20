/*
 * player.hpp
 *
 *  Created on: Dec 1, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <utility>

#include "player_proxy.hpp"
#include "protocol.hpp"

using namespace std;
using namespace boost::asio::ip;

namespace paxos {

/**
 * Abstract base class for the roles in Paxos algorithm, responsible for implementing communication mechanisms
 */
template <typename MsgT=typename protocol<>::message_type>
class player {
 public:
	player(int id, int port, const vector<pair<string, string> > &peers);
	virtual ~player() { }
	void run();
	string get_name() const;
 protected:
	virtual void handle_request(const string &raw_message, boost::shared_ptr<udp::endpoint> remote_endpoint) = 0;
	virtual string get_player_type() const { return "player"; }
	void send_message_to_all(const MsgT &message);
	void send_message_back(const MsgT &message, boost::shared_ptr<udp::endpoint> remote_endpoint);
	int id_;
	string get_id_string() const { ostringstream oss; oss << id_; return oss.str(); }
 private:
	boost::asio::io_service io_service_;
	int port_;
	udp::socket server_socket_;
	vector<boost::shared_ptr<player_proxy<MsgT> > > peers_;
	static const size_t buf_size = 1 << 10;
};

// implementation
template <typename MsgT>
player<MsgT>::player(int id, int port, const vector<pair<string, string> > &peers) :
	id_(id), port_(port), server_socket_(io_service_, udp::endpoint(udp::v4(), port)) {
	// connecting the peers
	for (size_t i = 0; i < peers.size(); ++i) {
		const pair<string, string> &info = peers[i];
		boost::shared_ptr<player_proxy<MsgT> > ptr(new player_proxy<MsgT>(info.first, info.second, io_service_));
		peers_.push_back(ptr);
	}
}

template <typename MsgT>
void player<MsgT>::run() {
	// main server loop
	try {
		while (true) {
			boost::array<char, buf_size> recv_buf;
			boost::shared_ptr<udp::endpoint> remote_endpoint(new udp::endpoint);
			boost::system::error_code error;
			size_t len = server_socket_.receive_from(boost::asio::buffer(recv_buf), *remote_endpoint, 0, error);
			if (error && error != boost::asio::error::message_size)
				throw boost::system::system_error(error);
			// launch new thread to handle request
			boost::thread(boost::bind(&player::handle_request, this, string(recv_buf.begin(), recv_buf.begin() + len), remote_endpoint));
		}
	} catch (exception &e) {
		cerr << "Exception in player::run(): " << e.what() << endl;
	}
}

template <typename MsgT>
string  player<MsgT>::get_name() const {
	ostringstream oss;
	oss << get_player_type() << "[id=" << id_ << "]";
	return oss.str();
}

template <typename MsgT>
void player<MsgT>::send_message_to_all(const MsgT &message) {
	for (size_t i = 0; i != peers_.size(); ++i) {
		peers_[i]->send_message(server_socket_, message);
	}
}

template <typename MsgT>
void player<MsgT>::send_message_back(const MsgT &message, boost::shared_ptr<udp::endpoint> remote_endpoint) {
	server_socket_.send_to(boost::asio::buffer(string(message)), *remote_endpoint);
}


} // namespace Paxos






