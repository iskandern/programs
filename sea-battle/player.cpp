#include "player.h"
#include <algorithm>

const bool player::cellExists(int xPos, int yPos) {
	if (xPos >= 0 && xPos < TABLE_SIZE && yPos >= 0 && yPos < TABLE_SIZE)
		return true;
	return false;
}

const int player::getShoots() {
	return shoots_;
}

const STATUS player::getStatus(int xPos, int yPos, TABLE table) {
	switch (table) {
		case PLAYER:
			return playerField_[xPos][yPos];

		case OPPONENT:
			return opponentField_[xPos][yPos];
	}
}

STATUS player::getStatusAndWriteHit(int xPos, int yPos) {
	if (playerField_[xPos][yPos] != SHIP)
		return playerField_[xPos][yPos];

	playerField_[xPos][yPos] = HIT;
	for (int dir = 0; dir < 4; ++dir) {
		int xNearPos = xPos + DIRECTIONS[dir][0];
		int yNearPos = yPos + DIRECTIONS[dir][1];

		if (!cellExists(xNearPos, yNearPos))
			continue;
		STATUS cellStatus = playerField_[xNearPos][yNearPos];

		if (cellStatus != HIT && cellStatus != SHIP)
			continue;

		int xDeviation = abs(xNearPos - xPos);
		int yDeviation = abs(yNearPos - yPos);
		STATUS posStatus;

		for (int i = 0; i < 2; ++i) {
			int xCheckPos = xPos;
			int yCheckPos = yPos;
			do {
				xCheckPos = xCheckPos + xDeviation;
				yCheckPos = yCheckPos + yDeviation;
				if (!cellExists(xCheckPos, yCheckPos))
					break;

				posStatus = playerField_[xCheckPos][yCheckPos];
				if (posStatus == SHIP)
					return HIT;

			} while (posStatus == HIT);

			xDeviation *= -1;
			yDeviation *= -1;
		}
		return KILL;
	}
	return KILL;
}

void player::placeShip(int xStartPos, int yStartPos, int shipCellNum, int direction) {
	for (int deviation = 0; deviation < shipCellNum; ++deviation) {
		int xShipCellPos = xStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][0];
		int yShipCellPos = yStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][1];
		
		for (int dir = 0; dir < 8; dir++) {
			int xPos = xShipCellPos + AREA_DIRECTIONS[dir][0];
			int yPos = yShipCellPos + AREA_DIRECTIONS[dir][1];
			if (cellExists(xPos, yPos) && getStatus(xPos, yPos, PLAYER) == EMPTY)
				playerField_[xPos][yPos] = SHIP_AREA;
		}
			

		playerField_[xShipCellPos][yShipCellPos] = SHIP;
	}
}

player::player(PLAYER_TYPE player) {
	srand(time(NULL));
	for (int i = 0; i < TABLE_SIZE; ++i) {
		for (int j = 0; j < TABLE_SIZE; ++j) {
			playerField_[i][j] = EMPTY;
			opponentField_[i][j] = EMPTY;
		}
	}
	for (int shipCellNum = 1; shipCellNum <= USING_SHIPS; ++shipCellNum) {
		ships_counter_[shipCellNum - 1] = 0;
		opponent_ships_counter_[shipCellNum - 1] = 0;
	}
		
	shoots_ = 0;
	isReadyToPlay_ = false;
	if (player == USER)
		return;

	for (int shipCellNum = USING_SHIPS; shipCellNum >= 1; --shipCellNum) {
		for (int shipsNum = 1; shipsNum <= SHIPS_NUMBERS[shipCellNum - 1]; ++shipsNum) {
			bool isReady = false;
			do {
				int xStartPos = rand() % (TABLE_SIZE - shipCellNum + 1);
				int yStartPos = rand() % (TABLE_SIZE - shipCellNum + 1);
				int direction = rand() % 2;
				isReady = true;

				for (int deviation = 0; deviation < shipCellNum; ++deviation) {
					int xPos = xStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][0];
					int yPos = yStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][1];
					if (getStatus(xPos, yPos, PLAYER) != EMPTY)
						isReady = false;
				}

				if (isReady) {
					placeShip(xStartPos, yStartPos, shipCellNum, direction);
					ships_counter_[shipCellNum - 1]++;
				}

			} while (!isReady);
		}
	}
	isReadyToPlay_ = true;
}

void player::writeShoot(STATUS status, int xPos, int yPos) {
	shoots_++;
	if (status == HIT) {
		isUnfinishedKill_ = true;
		xPrevHit_ = xPos;
		yPrevHit_ = yPos;
		for (int dir = 0; dir < 8; ++dir) {
			int xNearPos = xPos + AREA_DIRECTIONS[dir][0];
			int yNearPos = yPos + AREA_DIRECTIONS[dir][1];

			if (!cellExists(xNearPos, yNearPos) || (xNearPos == xPos) || (yNearPos == yPos))
				continue;

			opponentField_[xNearPos][yNearPos] = SHIP_AREA;
		}
		opponentField_[xPos][yPos] = HIT;
		return;
	}
	if (status == KILL) {
		isUnfinishedKill_ = false;
		opponentField_[xPos][yPos] = HIT;

		bool checkDirection = false;
		for (int dir = 0; dir < 8; ++dir) {
			int xNearPos = xPos + AREA_DIRECTIONS[dir][0];
			int yNearPos = yPos + AREA_DIRECTIONS[dir][1];

			if (!cellExists(xNearPos, yNearPos))
				continue;

			STATUS cellStatus = opponentField_[xNearPos][yNearPos];
			if (cellStatus != HIT) {
				opponentField_[xNearPos][yNearPos] = SHIP_AREA;
				continue;
			}

			if (checkDirection)
				continue;

			int xDeviation = abs(xNearPos - xPos);
			int yDeviation = abs(yNearPos - yPos);
			STATUS posStatus;
			int cellCount = 1;

			for (int i = 0; i < 2; ++i) {
				int xCheckPos = xPos;
				int yCheckPos = yPos;
				do {
					xCheckPos = xCheckPos + xDeviation;
					yCheckPos = yCheckPos + yDeviation;
					if (!cellExists(xCheckPos, yCheckPos))
						break;

					posStatus = opponentField_[xCheckPos][yCheckPos];
					if (posStatus == HIT) {
						cellCount++;
						int xLastShipPos = xCheckPos + xDeviation;
						int yLastShipPos = yCheckPos + yDeviation;
						if (cellExists(xLastShipPos, yLastShipPos) && opponentField_[xLastShipPos][yLastShipPos] != HIT)
							opponentField_[xLastShipPos][yLastShipPos] = SHIP_AREA;
					}
				} while (posStatus == HIT);

				xDeviation *= -1;
				yDeviation *= -1;
			}
			opponent_ships_counter_[cellCount - 1]++;
			checkDirection = true;
		}
		if (!checkDirection)
			opponent_ships_counter_[0]++;
		return;
	}
	opponentField_[xPos][yPos] = MISS;
}

const bool player::isFull() {
	for (int shipCellNum = 1; shipCellNum <= USING_SHIPS; ++shipCellNum)
		if (ships_counter_[shipCellNum - 1] != SHIPS_NUMBERS[shipCellNum - 1])
			return false;
	return true;
}

const bool player::isOpponentLost() {
	for (int shipCellNum = 1; shipCellNum <= USING_SHIPS; ++shipCellNum)
		if (opponent_ships_counter_[shipCellNum - 1] != SHIPS_NUMBERS[shipCellNum - 1])
			return false;
	return true;
}

const bool player::isReady() {
	return isReadyToPlay_;
}

bool player::setShipIfOk(int xStartPos, int yStartPos, int xEndPos, int yEndPos) {
	if (isReadyToPlay_ || !cellExists(xStartPos, yStartPos) || !cellExists(xEndPos, yEndPos))
		return false;
	
	int direction;
	int shipCellNum;
	if (xStartPos != xEndPos && yStartPos == yEndPos) {
		direction = 0;
		shipCellNum = abs(xStartPos - xEndPos) + 1;
		xStartPos = std::min(xStartPos, xEndPos);
	}
	else if (xStartPos == xEndPos && yStartPos != yEndPos) {
		direction = 1;
		shipCellNum = abs(yStartPos - yEndPos) + 1;
		yStartPos = std::min(yStartPos, yEndPos);
	}
	else if (xStartPos == xEndPos && yStartPos == yEndPos) {
		direction = 0;
		shipCellNum = 1;
	}
	else 
		return false;

	if (shipCellNum > MAX_SHIP_SIZE || ships_counter_[shipCellNum - 1] == SHIPS_NUMBERS[shipCellNum - 1])
		return false;

	bool isReady = true;
	for (int deviation = 0; deviation < shipCellNum; ++deviation) {
		int xPos = xStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][0];
		int yPos = yStartPos + deviation * PLACE_SHIP_DIRECTIONS[direction][1];
		if (getStatus(xPos, yPos, PLAYER) != EMPTY)
			isReady = false;
	}
	if (isReady) {
		placeShip(xStartPos, yStartPos, shipCellNum, direction);
		ships_counter_[shipCellNum - 1]++;
	}

	if (isFull())
		isReadyToPlay_ = true;
	return isReady;
}

const std::pair<int, int> player::getRandomCellToShoot() {
	srand(time(NULL));
	int xShootPos, yShootPos;

	do {
		xShootPos = rand() % TABLE_SIZE;
		yShootPos = rand() % TABLE_SIZE;
	} while (getStatus(xShootPos, yShootPos, OPPONENT) != EMPTY);

	return std::make_pair(xShootPos, yShootPos);
}

const std::pair<int, int> player::getCellToShoot() {
	srand(time(NULL));

	if (!isUnfinishedKill_)
		return getRandomCellToShoot();

	for (int dir = 0; dir < 4; ++dir) {
		int xPos = xPrevHit_ + DIRECTIONS[dir][0];
		int yPos = yPrevHit_ + DIRECTIONS[dir][1];

		if (!cellExists(xPos, yPos) || getStatus(xPos, yPos, OPPONENT) != HIT)
			continue;
		
		int lineDir = (rand() % 2) * 2 - 1;
		int xDeviation = abs(xPrevHit_ - xPos) * lineDir;
		int yDeviation = abs(yPrevHit_ - yPos) * lineDir;
		STATUS posStatus;

		for (int i = 0; i < 2; ++i) {
			int xCheckPos = xPos;
			int yCheckPos = yPos;
			do {
				xCheckPos = xCheckPos + xDeviation;
				yCheckPos = yCheckPos + yDeviation;
				if (!cellExists(xCheckPos, yCheckPos))
					break;

				posStatus = getStatus(xCheckPos, yCheckPos, OPPONENT);
				if (posStatus == MISS || posStatus == SHIP_AREA)
					break;

				if (posStatus == EMPTY)
					return std::make_pair(xCheckPos, yCheckPos);

			} while (true);

			xDeviation *= -1;
			yDeviation *= -1;
		}
		isUnfinishedKill_ = false;
		return getRandomCellToShoot();
	}

	for (int dir = 0; dir < 4; ++dir) {
		int xCheckPos = xPrevHit_ + DIRECTIONS[dir][0];
		int yCheckPos = yPrevHit_ + DIRECTIONS[dir][1];
		if (!cellExists(xCheckPos, yCheckPos))
			continue;

		if (getStatus(xCheckPos, yCheckPos, OPPONENT) == EMPTY)
			return std::make_pair(xCheckPos, yCheckPos);
	}

	isUnfinishedKill_ = false;
	return getRandomCellToShoot();
}


player::player() {
}

player::~player() {
}
