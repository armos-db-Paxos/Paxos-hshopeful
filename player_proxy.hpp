/*
 * player_proxy.hpp
 *
 *  Created on: Dec 2, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio::ip;

namespace paxos {

/**
 * Proxy pattern: Placeholder for remote player
 */
template <typename MsgT>
class player_proxy {
 public:
	player_proxy(const string &hn, const string &pt, boost::asio::io_service &io_service) :
		hostname(hn), port(pt), socket(io_service) {
		try {
			udp::resolver resolver(io_service);
			udp::resolver::query query(udp::v4(), hostname, port);
			receiver_endpoint = *resolver.resolve(query);
			socket.open(udp::v4());
		} catch (exception &e) {
			cerr << "Exception in player_proxy::player_proxy(): " << e.what() << endl;
		}
	}

	void send_message(udp::socket &server_socket, const MsgT &message) {
		server_socket.send_to(boost::asio::buffer(string(message)), receiver_endpoint);
	}
 private:
	string hostname;
	string port;
	udp::endpoint receiver_endpoint;
	udp::socket socket;
};


}





