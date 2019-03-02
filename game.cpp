#include "game.h"

game::game() {
	first_ = player(BOT);
	Sleep(1000);
	second_ = player(BOT);
	whoGoes_ = FIRST_PLAYER;
	firstType_ = BOT,
	secondType_ = BOT;
}

game::game(PLAYER_TYPE first, PLAYER_TYPE second) {
	first_ = player(first);
	Sleep(1000);
	second_ = player(second);
	whoGoes_ = FIRST_PLAYER;
	firstType_ = first,
	secondType_ = second;
}

game::~game()
{
}

PLAYER_TYPE game::getFirstType() {
	return firstType_;
}

PLAYER_TYPE game::getSecondType() {
	return secondType_;
}

PLAYER_NUMBER game::whoGoes() {
	return whoGoes_;
}

STATUS game::getCell(PLAYER_NUMBER player, int xPos, int yPos) {
	switch (player) {
		case FIRST_PLAYER:
			return first_.getStatus(xPos, yPos, PLAYER);
		case SECOND_PLAYER:
			return second_.getStatus(xPos, yPos, PLAYER);
	};
}

bool game::isReady() {
	return first_.isReady() && second_.isReady();
}

bool game::firstIsReady() {
	return first_.isReady();
}

bool game::secondIsReady() {
	return second_.isReady();
}

bool game::isFinish() {
	return first_.isOpponentLost() || second_.isOpponentLost();
}

bool game::move() {
	if (!first_.isReady() || !second_.isReady())
		return false;

	if (first_.isOpponentLost() || second_.isOpponentLost())
		return false;

	if (whoGoes_ == FIRST_PLAYER) {
		if (firstType_ != BOT)
			return false;

		std::pair<int, int> pos = first_.getCellToShoot();

		STATUS cellStatus = second_.getStatusAndWriteHit(pos.first, pos.second);
		first_.writeShoot(cellStatus, pos.first, pos.second);
		if (cellStatus != HIT && cellStatus != KILL)
			whoGoes_ = SECOND_PLAYER;
	}
	else if (whoGoes_ == SECOND_PLAYER) {
		if (secondType_ != BOT)
			return false;

		std::pair<int, int> pos = second_.getCellToShoot();

		STATUS cellStatus = first_.getStatusAndWriteHit(pos.first, pos.second);
		second_.writeShoot(cellStatus, pos.first, pos.second);
		if (cellStatus != HIT && cellStatus != KILL) {
			whoGoes_ = FIRST_PLAYER;
		}
	}
	return true;
}

bool game::move(PLAYER_NUMBER player, int xPos, int yPos) {
	if (!first_.isReady() || !second_.isReady()) 
		return false;
	
	if (first_.isOpponentLost() || second_.isOpponentLost())
		return false;

	if (whoGoes_ != player)
		return false;

	if (whoGoes_ == FIRST_PLAYER) {
		if (!first_.cellExists(xPos, yPos))
			return false;

		STATUS cellStatus = second_.getStatusAndWriteHit(xPos, yPos);
		first_.writeShoot(cellStatus, xPos, yPos);
		if (cellStatus != HIT && cellStatus != KILL)
			whoGoes_ = (whoGoes_ == FIRST_PLAYER) ? SECOND_PLAYER : FIRST_PLAYER;

		return true;
	}

	if (whoGoes_ == SECOND_PLAYER) {
		if (!second_.cellExists(xPos, yPos))
			return false;

		STATUS cellStatus = first_.getStatusAndWriteHit(xPos, yPos);
		second_.writeShoot(cellStatus, xPos, yPos);
		if (cellStatus != HIT && cellStatus != KILL)
			whoGoes_ = (whoGoes_ == FIRST_PLAYER) ? SECOND_PLAYER : FIRST_PLAYER;
		
		return true;
	}


	return false;
}

bool game::setShipIfOk(PLAYER_NUMBER player, int xStartPos, int yStartPos, int xEndPos, int yEndPos) {
	switch (player) {
		case FIRST_PLAYER:
			return first_.setShipIfOk(xStartPos, yStartPos, xEndPos, yEndPos);
		case SECOND_PLAYER:
			return second_.setShipIfOk(xStartPos, yStartPos, xEndPos, yEndPos);
	};
	return false;
}

int game::getShoots(PLAYER_NUMBER player) {
	switch (player) {
		case FIRST_PLAYER:
			return first_.getShoots();
		case SECOND_PLAYER:
			return second_.getShoots();
	};
	return false;
}