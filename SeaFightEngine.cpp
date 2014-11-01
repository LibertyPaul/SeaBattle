#include "SeaFightEngine.hpp"
#include <algorithm>
#include <random>
#include <chrono>


using namespace std;


SeaFightEngine::SeaFightEngine():
	myField(move(getRandomField())){

	for(auto &row : enemyField)
		for(auto &point : row)
			point = FieldState::unknown;
}


bool SeaFightEngine::isShip(const uint8_t row, const uint8_t col, const field_t &field) const{
	return
		field.at(row).at(col) == FieldState::ship ||
		field.at(row).at(col) == FieldState::wounded ||
		field.at(row).at(col) == FieldState::destroyed;
}

vector<pair<uint8_t, uint8_t>> SeaFightEngine::getPointsAroundTheShip(const Ship &ship) const{
	vector<pair<uint8_t, uint8_t>> result;

	switch(ship.orientation){
		case ShipOrientation::vertical:
			//заполняем поля, выше корабля
			if(ship.row >= 1)//если они есть
				for(uint8_t currentCol = max(0, ship.col - 1); currentCol <= min(fieldSize - 1, ship.col + 1); ++currentCol)
					result.push_back(pair<uint8_t, uint8_t>(ship.row - 1, currentCol));

			//поля слева и справа
			for(uint8_t i = 0; i < ship.length; ++i){
				if(ship.col >= 1)
					result.push_back(pair<uint8_t, uint8_t>(ship.row + i, ship.col - 1));
				if(ship.col + 1 < fieldSize)
					result.push_back(pair<uint8_t, uint8_t>(ship.row + i, ship.col + 1));
			}

			//поля снизу
			if(ship.row + ship.length < fieldSize)//если они есть
				for(uint8_t currentCol = max(0, ship.col - 1); currentCol <= min(fieldSize - 1, ship.col + 1); ++currentCol)
					result.push_back(pair<uint8_t, uint8_t>(ship.row + ship.length, currentCol));
			break;

		case ShipOrientation::horizontal:
			if(ship.row >= 1)
				for(uint8_t currentCol = max(0, ship.col - 1); currentCol <= min(fieldSize - 1, ship.col + ship.length); ++currentCol)
					result.push_back(pair<uint8_t, uint8_t>(ship.row - 1, currentCol));

			if(ship.col >= 1)
				result.push_back(pair<uint8_t, uint8_t>(ship.row, ship.col - 1));
			if(ship.col + ship.length < fieldSize)
				result.push_back(pair<uint8_t, uint8_t>(ship.row, ship.col + ship.length));


			if(ship.row + 1 < fieldSize)
				for(uint8_t currentCol = max(0, ship.col - 1); currentCol <= min(fieldSize - 1, ship.col + ship.length); ++currentCol)
					result.push_back(pair<uint8_t, uint8_t>(ship.row + 1, currentCol));

			break;

		default: throw runtime_error("Unknown ShipOrientation");
	}
	return result;
}





bool SeaFightEngine::isShipCrossingCurrentPosition(const Ship &ship, const field_t &currentPosition) const{
	int16_t currentRow = ship.row, currentCol = ship.col;
	for(uint8_t i = 0; i < ship.length; ++i){//для каждой клетки корабля проверяем наличие в ней или в соседних клетках других кораблей
		for(uint8_t row = max(0, currentRow - 1); row <= min(fieldSize - 1, currentRow + 1); ++row)//можно заменить на getPointsAroundTheShip
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
				default: throw runtime_error("Unknown ShipOrienatation value");
			}
	}
	return false;
}


void SeaFightEngine::insertShip(const Ship &ship, field_t &field) const{
	uint8_t currentRow = ship.row;
	uint8_t currentCol = ship.col;

	for(uint8_t i = 0; i < ship.length; ++i){
		if(field.at(currentRow).at(currentCol) != FieldState::free)
			throw runtime_error("insertShip error: point is not free");

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

void SeaFightEngine::removeShip(const Ship &ship, field_t &field) const{
	uint8_t currentRow = ship.row;
	uint8_t currentCol = ship.col;

	for(uint8_t i = 0; i < ship.length; ++i){
		if(isShip(currentRow, currentCol, field) == false)
			throw runtime_error("removeShip error: no ship over there");

		field.at(currentRow).at(currentCol) = FieldState::free;

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

void SeaFightEngine::destroyShip(const Ship &ship, field_t &field) const{
	uint8_t currentRow = ship.row;
	uint8_t currentCol = ship.col;

	for(uint8_t i = 0; i < ship.length; ++i){
		if(isShip(currentRow, currentCol, field) == false)
			throw runtime_error("removeShip error: no ship over there");

		field.at(currentRow).at(currentCol) = FieldState::destroyed;

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


field_t SeaFightEngine::getRandomField() const{
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

		if(success == false)//из за сложившейся позиции мы больше не можем добавлять корабли(вероятность сего события ~5%)
			return getRandomField();//рекурсивно повторяем до тех пор, пока не получится корректная расстановка.
			//тут был бы неплох алгоритм поиска с возвратом, но я боюсь, что из за него пострадает случайность расстановки.

	}
	return result;
}



Ship SeaFightEngine::getShipByPoint(const field_t &field, const uint8_t row, const uint8_t col) const{
	Ship result;

	//getting orientation

	result.orientation = ShipOrientation::vertical;//для однопалубного корабля

	if(row + 1 < fieldSize)
		if(isShip(row + 1, col, field))
			result.orientation = ShipOrientation::vertical;
	if(row - 1 >= 0)
		if(isShip(row - 1, col, field))
			result.orientation = ShipOrientation::vertical;
	if(col + 1 < fieldSize)
		if(isShip(row, col + 1, field))
			result.orientation = ShipOrientation::horizontal;
	if(col - 1 >= 0)
		if(isShip(row, col - 1, field))
			result.orientation = ShipOrientation::horizontal;
	//условия избыточны, можно выкинуть vertical


	//ищем самую верхнюю-правую точку

	switch(result.orientation){
		case ShipOrientation::vertical:{
			int16_t currentRow;
			for(currentRow = row; currentRow >= 0; --currentRow)
				if(isShip(currentRow, col, field) == false)
					break;
			result.row = currentRow + 1;//+ 1 хреново. но работает 100%
			result.col = col;
			break;
		}

		case ShipOrientation::horizontal:{
			int16_t currentCol;
			for(currentCol = col; currentCol >= 0; --currentCol)
				if(isShip(row, currentCol, field) == false)
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
				if(isShip(currentRow + 1, result.col, field))
					++result.length;
				else
					break;
			}
			break;

		case ShipOrientation::horizontal:
			for(uint8_t currentCol = result.col; currentCol < fieldSize - 1; ++currentCol){
				if(isShip(result.row, currentCol + 1, field))
					++result.length;
				else
					break;
			}
			break;
		default: throw runtime_error("Unknown value of ShipOrientation enum");
	}

	return result;
}


bool SeaFightEngine::isShipAlive(const Ship &ship) const{
	switch(ship.orientation){
		case ShipOrientation::vertical:
			for(uint8_t currentRow = ship.row; currentRow < ship.length + ship.row; ++currentRow)
				if(myField.at(currentRow).at(ship.col) == FieldState::ship)
					return true;
			break;

		case ShipOrientation::horizontal:
			for(uint8_t currentCol = ship.col; currentCol < ship.length + ship.col; ++currentCol)
				if(myField.at(ship.row).at(currentCol) == FieldState::ship)
					return true;
			break;

		default: throw runtime_error("Unknown value of ShipOrientation enum");
	}

	return false;
}



FieldState SeaFightEngine::applyEnemyTurn(const uint8_t row, const uint8_t col){
	switch(myField.at(row).at(col)){
		case FieldState::free:
			return FieldState::free;

		case FieldState::ship:{
			myField.at(row).at(col) = FieldState::wounded;
			Ship woundedShip = getShipByPoint(myField, row, col);
			if(isShipAlive(woundedShip) == false){
				destroyShip(woundedShip, myField);
				return FieldState::destroyed;
			}
			else
				return FieldState::wounded;
			break;
		}

		case FieldState::destroyed://так происходить не должно
			return FieldState::destroyed;

		default: throw runtime_error("Unknown FieldState enum value");
	}
}

bool SeaFightEngine::isLost() const{
	for(const auto &row : myField)
		for(const auto &point : row)
			if(point == FieldState::ship)
				return false;
	return true;
}

void SeaFightEngine::applyMyTurn(const uint8_t row, const uint8_t col, FieldState result){
	switch(result){
		case FieldState::destroyed:{//противник ответил "убил"
			enemyField.at(row).at(col) = FieldState::ship;//добавляем последнюю палубу
			Ship destroyedShip = getShipByPoint(enemyField, row, col);//вычислаем вектор корабля
			destroyShip(destroyedShip, enemyField);//уничтожаем

			//вокруг уничтоженного корабля все клетки пусты. Это надо отобразить на карте.

			auto pointsAround = move(getPointsAroundTheShip(destroyedShip));
			for(const auto &point : pointsAround)
				enemyField.at(point.first).at(point.second) = FieldState::free;

			break;
		}

		case FieldState::wounded://"ранил"
		case FieldState::free://"мимо"
			enemyField.at(row).at(col) = result;
			break;

		case FieldState::ship:
			throw runtime_error("Wrong answer");
		default:
			throw runtime_error("Unknown FieldState enum value");
	}
}

bool SeaFightEngine::isWon() const{
	/*auto ships = shipList;
	auto fieldCopy = enemyField;

	for(uint8_t row = 0; row < fieldSize; ++row)
		for(uint8_t col = 0; col < fieldSize; ++col)
			switch(enemyField.at(row).at(col)){
				case FieldState::ship:
					return false;

				case FieldState::destroyed:{
					Ship currentShip = getShipByPoint(fieldCopy, row, col);
					removeShip(currentShip, fieldCopy);

					auto shipTypeIt = find_if(ships.begin(), ships.end(), [&currentShip](const pair<uint8_t, uint8_t> &shipType){
						return shipType.first == currentShip.length;
					});

					if(shipTypeIt->second == 0)
						throw runtime_error("Ship count overflow");
					--shipTypeIt->second;

					break;
				}

				case FieldState::free:
				case FieldState::unknown:
				case FieldState::wounded:
					break;

				default: throw runtime_error("Unknown FieldState enum value");
			}

	auto aliveShips = find_if(ships.begin(), ships.end(), [](const pair<uint8_t, uint8_t> &shipType){
		return shipType.second != 0;
	});

	return aliveShips == ships.end();*/

	uint16_t count = 0;
	for(uint8_t row = 0; row < fieldSize; ++row)
		for(uint8_t col = 0; col < fieldSize; ++col)
			if(isShip(row, col, enemyField))
				++count;

	uint16_t shipPointSum = 0;
	for(const auto &shipType : shipList)
		shipPointSum += shipType.first * shipType.second;

	return count == shipPointSum;
}











const uint32_t layoutSize = 24;
typedef array<string, layoutSize> FieldLayout;

FieldLayout getFieldLayout(const field_t &field){
	const static uint32_t margin_left = 6, margin_top = 2, padding_left = 6, padding_top = 2;
	const static FieldLayout emptyLayout = {{
		{"    1   2   3   4   5   6   7   8   9  10  "},
		{"  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐"},
		{"A │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"B │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"C │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"D │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"E │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"F │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"G │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"H │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"I │   │   │   │   │   │   │   │   │   │   │"},
		{"  ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤"},
		{"J │   │   │   │   │   │   │   │   │   │   │"},
		{"  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘"},
		{""},
		{""}
	}};

	FieldLayout result(emptyLayout);

	for(uint8_t row = 0; row < fieldSize; ++row)
		for(uint8_t col = 0; col < fieldSize; ++col){
			wchar_t currentSymbol;
			switch(field.at(row).at(col)){
				case FieldState::ship:
					currentSymbol = L'@';	//L'█';
					break;
				case FieldState::destroyed:
					currentSymbol = L'#';
					break;
				case FieldState::unknown:
					currentSymbol = L'?';	//L'░';
					break;
				case FieldState::free:
					currentSymbol = L' ';
					break;
				case FieldState::wounded:
					currentSymbol = L'*';
					break;
				default: throw runtime_error("Unknown FieldState enum value");
			}
			result.at(row * padding_top + margin_top).at(col * padding_left + margin_left) = currentSymbol;
		}
	return result;
}





ostream &operator<<(ostream &o, const SeaFightEngine &sfc){
	/*

		  1   2   3   4   5   6   7   8   9  10							  1   2   3   4   5   6   7   8   9  10
		┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐						┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
	А	│   │   │ █ │ █ │   │   │ █ │ █ │   │   │					А	│   │   │   │   │   │   │   │   │   │   │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Б	│   │   │   │   │   │   │   │   │   │   │					Б	│   │   │   │ ░ │ ░ │ ░ │   │ ░ │ ░ │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	В	│   │   │   │ █ │   │   │   │   │ # │   │					В	│   │   │   │ ░ │ # │ ░ │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Г	│   │   │   │   │   │ █ │   │   │ # │   │					Г	│   │   │ ░ │ ░ │ ░ │ ░ │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Д	│   │   │   │   │   │ █ │   │   │ # │   │					Д	│   │   │ ░ │   │   │   │   │ ░ │ # │ ░ │
		├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤						├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
	Е	│   │   │   │   │   │   │   │   │   │   │					Е	│   │   │ ░ │   │   │   │   │ ░ │ ░ │ ░ │
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

	const string delimiter("\t\t\t\t");

	for(uint32_t i = 0; i < layoutSize; ++i)
		o << myFieldLayout.at(i) << delimiter << enemyFieldLayout.at(i) << endl;

	return o;
}


ostream &operator<<(ostream &o, const field_t &field){
	FieldLayout fl(move(getFieldLayout(field)));
	for(const auto row : fl)
		o << row << endl;
	return o;
}









