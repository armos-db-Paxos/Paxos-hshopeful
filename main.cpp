/*
 * main.cpp
 *
 *  Created on: Nov 30, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>

#include "player_factory.hpp"

using namespace std;
using namespace boost::asio::ip;


void setup_player(const string &type, int id, int mainport, const vector<pair<string, string> > &peers) {
	boost::shared_ptr<paxos::player<> > p = paxos::player_factory<>::get_player(type, id, mainport, peers);
	cout << p->get_name() << " set up" << endl;
	p->run();
	cout << p->get_name() << " terminated" << endl;
}

void setup_client(const string &hostname, const string &port) {
	try {
		boost::asio::io_service io_service;
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), hostname, port);
		udp::endpoint receiver_endpoint = *resolver.resolve(query);
		udp::socket socket(io_service);
		socket.open(udp::v4());
		socket.send_to(boost::asio::buffer(string(paxos::protocol<>::get_client_request("hello, world!"))), receiver_endpoint);
		boost::array<char, 128> recv_buf;
		udp::endpoint sender_endpoint;
		size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
		cout << "Client gets: ";
		cout.write(recv_buf.data(), len);
		cout << endl;
	} catch (exception &e) {
		cerr << "Exception in run_client(): " << e.what() << endl;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cerr << "Usage: ./Paxos [proposer, acceptor, learner, or paxos_player] config_file_name | ./Paxos client hostname port" << endl;
		return 0;
	}
	// parse command-line parameters
	// node_type = "server" | "client"
	string node_type(argv[1]);
	// if node_type == "server"
	if (node_type == "proposer" || node_type == "acceptor" || node_type == "learner" || node_type == "paxos_player") {
		string type(argv[1]);
		string file_name(argv[2]);
		ifstream ifs(file_name.c_str());
		int id, mainport;
		ifs >> id >> mainport;
		string hostname, port;
		vector<pair<string, string> > peers;
		while (ifs >> hostname >> port) {
			peers.push_back(make_pair(hostname, port));
		}
		ifs.close();
		// launch the player
		boost::thread player_thread(boost::bind(setup_player, type, id, mainport, peers));
		player_thread.join();
	} else if (node_type == "client") {
		string hostname(argv[2]);
		string port(argv[3]);
		// launch the client
		boost::thread client_thread(boost::bind(setup_client, hostname, port));
		client_thread.join();
	} else {
		cerr << "Usage: ./Paxos [proposer, acceptor, learner, or paxos_player] config_file_name | ./Paxos client hostname port" << endl;
	}

	return 0;
}










