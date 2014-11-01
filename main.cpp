#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "GameManager.hpp"

using namespace std;


void man(const string &progName){
	cout << "Usage: " << endl;
	cout << progName << " <server 'port'>|<client 'ip' 'port'>" << endl;
}

int main(const int argc, const char **argv){
	string progName(argv[0]);

	if(argc < 2){
		man(progName);
		return -1;
	}

	string type(argv[1]);

	if(type == "server"){
		if(argc < 3){
			man(progName);
			return -1;
		}
		string port_s(argv[2]);
		uint16_t port = stoi(port_s);


		int mainSock_d = socket(AF_INET, SOCK_STREAM, 0);
		if(mainSock_d == -1)
			throw runtime_error("Socket openning error" + string(strerror(errno)));

		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));

		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = INADDR_ANY;

		int res = bind(mainSock_d, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
		if(res == -1)
			throw runtime_error("Bind error " + string(strerror(errno)));

		res = listen(mainSock_d, 1);
		if(res == -1)
			throw runtime_error("Listen error " + string(strerror(errno)));

		cout << "Server successfully started" << endl;



		GameManager gm(mainSock_d, NetworkType::server);
		gm.startGame();
	}
	else if(type == "client"){
		if(argc < 4){
			man(progName);
			return -1;
		}

		string ip(argv[2]);
		string port_s(argv[3]);
		uint16_t port = stoi(port_s);

		int sock_d = socket(AF_INET, SOCK_STREAM, 0);
		if(sock_d == -1)
			throw runtime_error(string("Socket creating error: ") + string(strerror(errno)));

		sockaddr_in sock_in;
		memset(&sock_in, 0, sizeof(sockaddr_in));
		sock_in.sin_family = AF_INET;
		sock_in.sin_port = htons(port);

		int res = inet_aton(ip.c_str(), reinterpret_cast<in_addr *>(&sock_in.sin_addr.s_addr));
		if(res == 0)
			throw runtime_error("inet_aton error " + to_string(errno));

		//cout << "Подключение к серверу " << ip << ":" << port << endl;

		res = connect(sock_d, reinterpret_cast<sockaddr *>(&sock_in), sizeof(sock_in));
		if(res == -1)
			throw runtime_error("Connect error " + string(strerror(errno)));

		cout << "Подключен к серверу" << endl;
		GameManager gm(sock_d, NetworkType::client);
		gm.startGame();
	}
	else{
		man(progName);
		return -1;
	}
	return 0;
}



