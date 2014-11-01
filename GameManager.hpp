#ifndef GameManager_HPP_INCLUDED
#define GameManager_HPP_INCLUDED

#include "SeaFightEngine.hpp"
#include "Buffer.hpp"
#include <iostream>
#include <array>
#include <utility>

enum class Command{
	turn,
	turnResult,
    newGame,
    endGame
};

enum class NetworkType{
	server,
	client
};

class GameManager{
	SeaFightEngine gameEngine;

	static constexpr uint32_t bufferSize = 1500;
	int sock_d;
	Buffer mainBuffer;
	NetworkType nt;



	void showCurrentGame() const;
	std::string readUserCommand() const;
	void applyCommand(const std::string &command) const;
	std::pair<uint8_t, uint8_t> getCoords(const std::string &coord) const;


	void sendBuffer();
	void waitForMessage();

	bool myTurn();
	bool enemyTurn();

public:
	GameManager(const int mainSock_d, const NetworkType nt);

	void startGame();
};



#endif // GameManager_HPP_INCLUDED
