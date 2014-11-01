#ifndef SEAFIGHTCLIENT_HPP_INCLUDED
#define SEAFIGHTCLIENT_HPP_INCLUDED

#include <array>
#include <vector>
#include <ostream>
#include <utility>

enum class FieldState{
	unknown,
	free,
	ship,
	wounded,
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

class SeaFightEngine{
	field_t myField;
	field_t enemyField;

	bool isShip(const uint8_t row, const uint8_t col, const field_t &field) const;//проверяет принадлежность клетки к кораблю(ship/wounded/destroyed)
	bool isShipCrossingCurrentPosition(const Ship &ship, const field_t &currentPosition) const;
	std::vector<std::pair<uint8_t, uint8_t>> getPointsAroundTheShip(const Ship &ship) const;

	void insertShip(const Ship &ship, field_t &field) const;
	void destroyShip(const Ship &ship, field_t &field) const;
	void removeShip(const Ship &ship, field_t &field) const;//3 функции практически идентичны. Можно создать общую абстракцию типа fillShip(ship, field, value, errorValue);

	Ship getShipByPoint(const field_t &field, const uint8_t row, const uint8_t col) const;
	bool isShipAlive(const Ship &ship) const;//is there any FieldState::ship points in the ship


	field_t getRandomField() const;
	bool isFieldValid() const;

public:
	SeaFightEngine();
	SeaFightEngine(const field_t &field);//will check(!) and set up position


	FieldState applyEnemyTurn(const uint8_t row, const uint8_t col);
	bool isLost() const;//проверка поражения

	void applyMyTurn(const uint8_t row, const uint8_t col, FieldState result);
	bool isWon() const;//проверка победы

	friend std::ostream &operator<<(std::ostream &o, const SeaFightEngine &sfc);
};

std::ostream &operator<<(std::ostream &o, const SeaFightEngine &sfc);//prints 2 fields with current game position, stats, etc.

std::ostream &operator<<(std::ostream &o, const field_t &field);




#endif // SEAFIGHTCLIENT_HPP_INCLUDED
