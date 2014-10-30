#ifndef SEAFIGHTCLIENT_HPP_INCLUDED
#define SEAFIGHTCLIENT_HPP_INCLUDED

#include <array>
#include <ostream>
#include <utility>

enum class FieldState{
	unknown,
	free,
	ship,
	destroyed
};

enum class ShipOrientation{
	vertical,
	horizontal
};

struct Ship{
	uint8_t row, col;
	uint8_t length;
	ShipOrientation orientation;
};

const uint16_t fieldSize = 10;

typedef std::array<std::array<FieldState, fieldSize>, fieldSize> field_t;

const std::array<std::pair<uint8_t, uint8_t>, 4> shipList = {{
	{4, 1},//1 четырехпалубный
	{3, 2},//2 трехпалубных
	{2, 3},//и т.д.
	{1, 4}
}};

class SeaFightClient{
	field_t myField;
	field_t enemyField;


	bool isShipCrossingCurrentPosition(const Ship &ship, const field_t &currentPosition) const;
	void insertShip(const Ship &ship, field_t &field) const;
	field_t getRandomField() const;
	bool isFieldValid() const;

	Ship getShipByPoint(const field_t &field, const uint8_t row, const uint8_t col) const;
	bool isShipDestroyed(const Ship &ship) const;


public:
	SeaFightClient();
	SeaFightClient(const field_t &field);//will check(!) and set up position


	FieldState applyEnemyTurn(const uint8_t row, const uint8_t col);
	bool isLost() const;//проверка поражения

	void applyMyTurn(const uint8_t row, const uint8_t col, FieldState result);
	bool isWon() const;//проверка победы

	friend std::wostream &operator<<(std::wostream &o, const SeaFightClient &sfc);
};

std::wostream &operator<<(std::wostream &o, const SeaFightClient &sfc);//prints 2 fields with current game position, stats, etc.

std::wostream &operator<<(std::wostream &o, const field_t &field);




#endif // SEAFIGHTCLIENT_HPP_INCLUDED
