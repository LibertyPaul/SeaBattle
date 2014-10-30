#include "SeaFightClient.hpp"
#include <algorithm>
#include <random>
#include <chrono>


using namespace std;


SeaFightClient::SeaFightClient():
	myField(move(getRandomField())){

	for(auto &row : enemyField)
		for(auto &point : row)
			point = FieldState::unknown;
}



bool SeaFightClient::isShipCrossingCurrentPosition(const Ship &ship, const field_t &currentPosition) const{
	int16_t currentRow = ship.row, currentCol = ship.col;
	for(uint8_t i = 0; i < ship.length; ++i){//для каждой клетки корабля проверяем наличие в ней или в соседних клетках других кораблей
		for(uint8_t row = max(0, currentRow - 1); row <= min(fieldSize - 1, currentRow + 1); ++row)
			for(uint8_t col = max(0, currentCol - 1); col <= min(fieldSize - 1, currentCol + 1); ++col)
				if(currentPosition.at(row).at(col) != FieldState::free)
					return true;

			switch(ship.orientation){
				case ShipOrientation::vertical:
					++currentRow;
					break;
				case ShipOrientation::horizontal:
					++currentCol;
					break;
				default: throw runtime_error("Unknown enum ShipOrienatation value");
			}
	}
	return false;
}



void SeaFightClient::insertShip(const Ship &ship, field_t &field) const{
	uint8_t currentRow = ship.row;
	uint8_t currentCol = ship.col;

	for(uint8_t i = 0; i < ship.length; ++i){
		field.at(currentRow).at(currentCol) = FieldState::ship;

		switch(ship.orientation){
			case ShipOrientation::vertical:
				++currentRow;
				break;
			case ShipOrientation::horizontal:
				++currentCol;
				break;
			default: throw runtime_error("Unknown enum ShipOrienatation value");
		}
	}
}




field_t SeaFightClient::getRandomField() const{
	vector<uint8_t> plainShipList;//список кораблей в виде 4,3,3,2,2,2,1,1,1,1
	for(const auto shipType : shipList)
		for(uint8_t i = 0; i < shipType.second; ++i)
			plainShipList.push_back(shipType.first);

	uint32_t seed = chrono::system_clock::now().time_since_epoch().count();
	shuffle(plainShipList.begin(), plainShipList.end(), default_random_engine(seed));//перетасовываем список кораблей

	field_t result;
	for(auto &row : result)
		for(auto &point : row)
			point = FieldState::free;

	mt19937 rg(seed);//optimize this shit
	for(const auto currentShipLength : plainShipList){
		ShipOrientation currentShipOrientation;
		if(rg() % 2 == 0)
			currentShipOrientation = ShipOrientation::vertical;
		else
			currentShipOrientation = ShipOrientation::horizontal;


		uint8_t maxRow, maxCol;//в зависимости от ориентации мы сужаем диапазон возможных координат первой клетки корабля
		switch(currentShipOrientation){
			case ShipOrientation::vertical:
				maxRow = fieldSize - currentShipLength;
				maxCol = fieldSize - 1;
				break;
			case ShipOrientation::horizontal:
				maxRow = fieldSize - 1;
				maxCol = fieldSize - currentShipLength;
				break;
			default: throw runtime_error("Unknown enum ShipOrienatation value");
		}

		vector<pair<uint8_t, uint8_t>> availablePoints;
		for(uint8_t row = 0; row <= maxRow; ++row)//собираем список свободных клеток
			for(uint8_t col = 0; col <= maxCol; ++col)
				if(result.at(row).at(col) == FieldState::free)
					availablePoints.push_back(pair<uint8_t, uint8_t>(row, col));

		shuffle(availablePoints.begin(), availablePoints.end(), default_random_engine(seed));//тасуем список свободных клеток и по порядку пытаемся вставить наш корабль

		bool success = false;

		Ship newShip;
		newShip.orientation = currentShipOrientation;
		newShip.length = currentShipLength;
		for(const auto coord : availablePoints){
			newShip.row = coord.first;
			newShip.col = coord.second;
			if(isShipCrossingCurrentPosition(newShip, result) == false){
				insertShip(newShip, result);
				success = true;
				break;
			}
		}

		if(success == false)//из за сложившейся позиции мы больше не можем добавлять корабли
			return getRandomField();//рекурсивно повторяем до тех пор, пока не получится корректная расстановка.
			//тут был бы неплох алгоритм поиска с возвратом, но я боюсь, что из за него пострадает случайность расстановки.

	}
	return result;
}



Ship SeaFightClient::getShipByPoint(const field_t &field, const uint8_t row, const uint8_t col) const{
	Ship result;

	//getting orientation

	result.orientation = ShipOrientation::vertical;//для однопалубного корабля

	if(row + 1 < fieldSize)
		if(
			field.at(row + 1).at(col) == FieldState::ship ||
			field.at(row + 1).at(col) == FieldState::destroyed
		)
		result.orientation = ShipOrientation::vertical;
	if(row - 1 >= 0)
		if(
			field.at(row - 1).at(col) == FieldState::ship ||
			field.at(row - 1).at(col) == FieldState::destroyed
		)
		result.orientation = ShipOrientation::vertical;
	if(col + 1 < fieldSize)
		if(
			field.at(row).at(col + 1) == FieldState::ship ||
			field.at(row).at(col + 1) == FieldState::destroyed
		)
		result.orientation = ShipOrientation::horizontal;
	if(col - 1 >= 0)
		if(
			field.at(row).at(col - 1) == FieldState::ship ||
			field.at(row).at(col - 1) == FieldState::destroyed
		)
		result.orientation = ShipOrientation::horizontal;
	//условия избыточны, можно выкинуть vertical


	//ищем самую верхнюю-правую точку

	switch(result.orientation){
		case ShipOrientation::vertical:{
			int16_t currentRow;
			for(currentRow = row; currentRow >= 0; --currentRow)
				if(
					field.at(currentRow).at(col) != FieldState::ship &&
					field.at(currentRow).at(col) != FieldState::destroyed
				)
				break;
			result.row = currentRow + 1;//+ 1 хреново. но работает 100%
			result.col = col;
			break;
		}

		case ShipOrientation::horizontal:{
			int16_t currentCol;
			for(currentCol = col; currentCol >= 0; --currentCol)
				if(
					field.at(row).at(currentCol) != FieldState::ship &&
					field.at(row).at(currentCol) != FieldState::destroyed
				)
				break;
			result.col = currentCol + 1;
			result.row = row;
			break;
		}

		default: throw runtime_error("Unknown value of ShipOrientation enum");
	}


	//считаем длину корабля

	result.length = 1;
	switch(result.orientation){
		case ShipOrientation::vertical:
			for(uint8_t currentRow = result.row; currentRow < fieldSize - 1; ++currentRow){
				if(
					field.at(currentRow + 1).at(result.col) != FieldState::ship &&
					field.at(currentRow + 1).at(result.col) != FieldState::destroyed
				)
				break;
				else
					++result.length;
			}
			break;

		case ShipOrientation::horizontal:
			for(uint8_t currentCol = result.col; currentCol < fieldSize - 1; ++currentCol){
				if(
					field.at(result.row).at(currentCol + 1) != FieldState::ship &&
					field.at(result.row).at(currentCol + 1) != FieldState::destroyed
				)
				break;
				else
					++result.length;
			}
			break;
		default: throw runtime_error("Unknown value of ShipOrientation enum");
	}

	return result;
}


bool SeaFightClient::isShipDestroyed(const Ship &ship) const{
	switch(ship.orientation){
		case ShipOrientation::vertical:
			for(uint8_t currentRow = ship.row; currentRow < ship.length + ship.row; ++currentRow)
				if(myField.at(currentRow).at(ship.col) == FieldState::ship)
					return false;
			break;

		case ShipOrientation::horizontal:
			for(uint8_t currentCol = ship.col; currentCol < ship.length + ship.col; ++currentCol)
				if(myField.at(ship.row).at(currentCol) == FieldState::ship)
					return false;
			break;

		default: throw runtime_error("Unknown value of ShipOrientation enum");
	}

	return true;
}



FieldState SeaFightClient::applyEnemyTurn(const uint8_t row, const uint8_t col){
	switch(myField.at(row).at(col)){
		case FieldState::free:
			return FieldState::free;

		case FieldState::ship:{
			myField.at(row).at(col) = FieldState::destroyed;
			Ship woundedShip = getShipByPoint(myField, row, col);
			if(isShipDestroyed(woundedShip))
				return FieldState::destroyed;
			else
				return FieldState::ship;
			break;
		}

		case FieldState::destroyed://так происходить не должно
			return FieldState::destroyed;

		default: throw runtime_error("Unknown FieldState enum value");
	}
}

bool SeaFightClient::isLost() const{
	for(const auto &row : myField)
		for(const auto &point : row)
			if(point == FieldState::ship)
				return false;
	return true;
}

void SeaFightClient::applyMyTurn(const uint8_t row, const uint8_t col, FieldState result){
	enemyField.at(row).at(col) = result;
}

bool SeaFightClient::isWon() const{
	auto ships = shipList;
	auto fieldCopy = enemyField;


	return false;
}































const uint32_t layoutSize = 24;
typedef array<wstring, layoutSize> FieldLayout;

FieldLayout getFieldLayout(const field_t &field){
	const static uint32_t margin_left = 4, margin_top = 2, padding_left = 4, padding_top = 2;
	const static FieldLayout emptyLayout = {{
		{L"    1   2   3   4   5   6   7   8   9  10  "},
		{L"  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐"},
		{L"A │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"B │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"C │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"D │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"E │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"F │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"G │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"H │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"I │   │   │   │   │   │   │   │   │   │   │"},
		{L"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{L"J │   │   │   │   │   │   │   │   │   │   │"},
		{L"  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘"},
		{L""},
		{L""}
	}};

	FieldLayout result(emptyLayout);

	for(uint8_t row = 0; row < fieldSize; ++row)
		for(uint8_t col = 0; col < fieldSize; ++col)
			switch(field.at(row).at(col)){
				case FieldState::ship:
					result.at(row * padding_top + margin_top).at(col * padding_left + margin_left) = L'@';	//L'█';
					break;
				case FieldState::destroyed:
					result.at(row * padding_top + margin_top).at(col * padding_left + margin_left) = L'#';
					break;
				case FieldState::unknown:
					result.at(row * padding_top + margin_top).at(col * padding_left + margin_left) = L'?';	//L'░';
					break;
				case FieldState::free:
					result.at(row * padding_top + margin_top).at(col * padding_left + margin_left) = L' ';
					break;
				default: throw runtime_error("Unknown FieldState enum value");
			}
	return result;
}





wostream &operator<<(wostream &o, const SeaFightClient &sfc){
	/*

		  1   2   3   4   5   6   7   8   9  10							  1   2   3   4   5   6   7   8   9  10
		┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐						┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
	А	│   │   │ █ │ █ │   │   │ █ │ █ │   │   │					А	│   │   │   │   │   │   │   │   │   │   │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Б	│   │   │   │   │   │   │   │ ░ │ ░ │ ░ │					Б	│   │   │   │ ░ │ ░ │ ░ │   │ ░ │ ░ │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	В	│   │   │   │ █ │   │   │   │ ░ │ # │ ░ │					В	│   │   │   │ ░ │ # │ ░ │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Г	│   │   │   │   │   │ █ │   │ ░ │ # │ ░ │					Г	│   │   │ ░ │ ░ │ ░ │ ░ │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Д	│   │   │   │   │   │ █ │   │ ░ │ # │ ░ │					Д	│   │   │ ░ │   │   │   │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Е	│   │   │   │   │   │   │   │ ░ │ ░ │ ░ │					Е	│   │   │ ░ │   │   │   │   │ ░ │ ░ │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Ж	│   │   │   │ █ │ █ │ █ │ █ │   │   │ █ │					Ж	│   │ ░ │   │   │   │   │   │   │   │   │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	З	│   │   │   │   │   │   │   │   │   │   │					З	│ ░ │   │   │   │   │   │   │   │   │   │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	И	│   │   │ █ │ █ │   │   │   │ █ │ █ │ █ │					И	│   │   │   │   │   │   │   │   │   │   │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	К	│   │   │   │   │   │   │   │   │   │   │					К	│   │   │   │   │   │   │   │   │   │   │
		└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘						└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

		Целые корабли: 1*4, 1*3, 3*2, 4*1								 Целые корабли: 1*4, 1*3, 3*2, 3*1

	*/

	FieldLayout myFieldLayout(move(getFieldLayout(sfc.myField)));
	FieldLayout enemyFieldLayout(move(getFieldLayout(sfc.enemyField)));

	const wstring delimiter(L"\t\t\t\t");

	for(uint32_t i = 0; i < layoutSize; ++i)
		o << myFieldLayout.at(i) << delimiter << enemyFieldLayout.at(i) << endl;

	return o;
}


wostream &operator<<(wostream &o, const field_t &field){
	FieldLayout fl(move(getFieldLayout(field)));
	for(const auto row : fl)
		o << row << endl;
	return o;
}









