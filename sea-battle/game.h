#pragma once

#include "player.h"
#include <windows.h>
#include <vector>

typedef enum { FIRST_PLAYER, SECOND_PLAYER, NULL_PLAYER } PLAYER_NUMBER;

class game {
public:
	game();
	game(PLAYER_TYPE first, PLAYER_TYPE second);
	~game();

	bool isReady();
	bool firstIsReady();
	bool secondIsReady();
	bool isFinish();
	bool move();
	PLAYER_NUMBER whoGoes();
	PLAYER_TYPE getFirstType();
	PLAYER_TYPE getSecondType();
	int getShoots(PLAYER_NUMBER player);
	bool move(PLAYER_NUMBER player, int xPos, int yPos);
	bool setShipIfOk(PLAYER_NUMBER player, int xStartPos, int yStartPos, int xEndPos, int yEndPos);
private:
	friend void draw(HWND hwnd);
	STATUS getCell(PLAYER_NUMBER player, int xPos, int yPos);

	player first_, second_;
	PLAYER_TYPE firstType_, secondType_;
	PLAYER_NUMBER whoGoes_;
};

