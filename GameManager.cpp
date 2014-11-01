#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#include <algorithm>
#include <iostream>


#include "GameManager.hpp"

using namespace std;


GameManager::GameManager(const int mainSock_d, const NetworkType nt):
	mainBuffer(bufferSize), nt(nt){

	mainBuffer.dropPointers();
	if(nt == NetworkType::server){
		cout << "Ожидание подключения" << endl;

		sockaddr_in clientAddr;
		socklen_t sl;
		memset(&clientAddr, 0, sizeof(clientAddr));
		memset(&sl, 0, sizeof(sl));

		sock_d = accept(mainSock_d, reinterpret_cast<sockaddr *>(&clientAddr), &sl);
		if(sock_d == -1)
			throw runtime_error("accept error: " + string(strerror(errno)));

		cout << "Входящее подключение" << endl << "Ожидание запроса начала новой игры" << endl;


		waitForMessage();
		Command request = mainBuffer.read<Command>();
		if(request != Command::newGame)
			throw runtime_error("incorrect command");
	}
	else if(nt == NetworkType::client){
		sock_d = mainSock_d;
		cout << "starting new game" << endl;

		mainBuffer.write<Command>(Command::newGame);
		sendBuffer();

	}
}

void GameManager::sendBuffer(){
	ssize_t res = send(sock_d, mainBuffer.get(), bufferSize, 0);
	if(res == -1)
		throw runtime_error("Sending error" + string(strerror(errno)));
}

void GameManager::waitForMessage(){
	mainBuffer.dropPointers();
	int res = recv(sock_d, mainBuffer.get(), bufferSize, 0);
	if(res == -1)
		throw runtime_error("waitForMessage error " + string(strerror(errno)));
}









void GameManager::showCurrentGame() const{
	cout << gameEngine << endl;
}

string GameManager::readUserCommand() const{
	string command;
	cin >> command;
	return command;
}

pair<uint8_t, uint8_t> GameManager::getCoords(const string &coord) const{
	constexpr static std::array<wchar_t, 10> letters = {{L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J'}};
	pair<uint8_t, uint8_t> result;

	wchar_t letter = coord.front();
	auto letterPos = find(letters.cbegin(), letters.cend(), letter);
	if(letterPos == letters.end())
		throw runtime_error("Invalid letter: " + to_string(letter));

	result.first = letterPos - letters.begin();

	string number_str = coord.substr(1);
	result.second = stoi(number_str) - 1;
	if(result.second >= fieldSize)
		throw runtime_error("Invalid number: " + to_string(letter));

	return result;
}

bool GameManager::myTurn(){
	pair<uint8_t, uint8_t> coords;
	bool correct;
	do{
		correct = true;
		string cmd = readUserCommand();
		try{
			coords = getCoords(cmd);
		}
		catch(exception &e){
			correct = false;
			cout << e.what() << endl;
		}
	}while(correct == false);

	mainBuffer.dropPointers();
	mainBuffer.write<Command>(Command::turn);
	mainBuffer.write<uint8_t>(coords.first);
	mainBuffer.write<uint8_t>(coords.second);

	sendBuffer();
	waitForMessage();

	Command res = mainBuffer.read<Command>();
	if(res == Command::endGame)
		return false;

	if(res == Command::turnResult){
		FieldState result = mainBuffer.read<FieldState>();
		gameEngine.applyMyTurn(coords.first, coords.second, result);

		switch(result){
			case FieldState::free:
				return false;

			case FieldState::wounded:
			case FieldState::destroyed:
				return true;

			default: throw runtime_error("incorrect responce");
		}
	}
	else
		throw runtime_error("unknown responce");
}



bool GameManager::enemyTurn(){
	waitForMessage();
	Command res = mainBuffer.read<Command>();

	switch(res){
		case Command::endGame://
		case Command::newGame://
		case Command::turnResult:
			throw runtime_error("incorrect responce");

		case Command::turn:{
			pair<uint8_t, uint8_t> coords;
			coords.first = mainBuffer.read<uint8_t>();
			coords.second = mainBuffer.read<uint8_t>();

			FieldState result = gameEngine.applyEnemyTurn(coords.first, coords.second);

			mainBuffer.dropPointers();
			mainBuffer.write<Command>(Command::turnResult);
			mainBuffer.write<FieldState>(result);
			sendBuffer();

			switch(result){
				case FieldState::destroyed:
				case FieldState::wounded:
					return true;

				case FieldState::free:
					return false;
				default: throw runtime_error("incorrect result");
			}
			break;
		}
		default: runtime_error("unknown responce");
	}
}






void GameManager::startGame(){
	bool repeat;
	showCurrentGame();
	if(nt == NetworkType::server)
		do{
			cout << "Ваш ход: >";
			repeat = myTurn();
			showCurrentGame();
			if(gameEngine.isWon()){
				cout << "You won" << endl;
				cin.get();
				return;
			}
		}while(repeat);

	showCurrentGame();

	while(true){
		do{
			cout << "Ожидание хода противника" << endl;
			repeat = enemyTurn();
			cout << "походил" << endl;
			showCurrentGame();
			if(gameEngine.isLost()){
				cout << "You lost" << endl;
				cin.get();
				return;
			}
		}while(repeat);

		do{
			cout << "Ваш ход: >";
			repeat = myTurn();
			showCurrentGame();
			if(gameEngine.isWon()){
				cout << "You won" << endl;
				cin.get();
				return;
			}
		}while(repeat);

		showCurrentGame();
	}
}













