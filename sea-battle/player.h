#pragma once

#include <iostream>
#include <ctime>
#include <math.h>

typedef enum { EMPTY, SHIP, SHIP_AREA, HIT, MISS, KILL, } STATUS;
typedef enum { PLAYER, OPPONENT } TABLE;
typedef enum { USER, BOT } PLAYER_TYPE;

const int TABLE_SIZE = 10;

const int USING_SHIPS = 4;
const int MAX_SHIP_SIZE = 4;
// for 1 cell ship first value, for 2 cell ship - second 
const int SHIPS_NUMBERS[USING_SHIPS] = { 4, 3, 2, 1 };
const int PLACE_SHIP_DIRECTIONS[2][2] = { {1, 0}, {0, 1} };
const int DIRECTIONS[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
const int AREA_DIRECTIONS[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

class player {
public:
	player();
	player(PLAYER_TYPE player);
	bool setShipIfOk(int xStartPos, int yStartPos, int xEndPos, int yEndPos);
	const STATUS getStatus(int xPos, int yPos, TABLE table);
	STATUS getStatusAndWriteHit(int xPos, int yPos);
	void writeShoot(STATUS status, int xPos, int yPos);
	const std::pair<int, int> getCellToShoot();
	const bool cellExists(int xPos, int yPos);
	const bool isFull();
	const bool isOpponentLost();
	const bool isReady();
	const int getShoots();
	~player();

private:
	STATUS playerField_[10][10];
	STATUS opponentField_[10][10];
	bool isUnfinishedKill_;
	bool isReadyToPlay_;
	int shoots_;
	int xPrevHit_, yPrevHit_;
	int ships_counter_[USING_SHIPS];
	int opponent_ships_counter_[USING_SHIPS];
	
	void placeShip(int xStartPos, int yStartPos, int shipCellNum, int direction);
	const std::pair<int, int> getRandomCellToShoot();

};

