#include <iostream>
#include "SeaFightClient.hpp"

using namespace std;

int main(){
	SeaFightClient sfc;
	wcout << sfc << endl;

	for(uint8_t row = 0; row < fieldSize / 2; ++row)
		for(uint8_t col = 0; col < fieldSize; ++col)
			sfc.applyEnemyTurn(row, col);


	wcout << sfc << endl;

}
