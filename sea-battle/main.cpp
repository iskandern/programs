#ifndef UNICODE
#define UNICODE
#endif 

/*
information:
to choose a player set BOT or USER in game(*, *) instead of stars
to see all tables set CLEAN_VIEW = true
to set ship: first left mouse click gives first ship position
			 second left mouse click gives second ship position
			 enter gives placing ship
set a ship example: choose A1, choose A4, enter => place a ship on A1, A2, A3, A4
to shoot ship: left mouse click gives remebering position
			   enter gives shooting
when both players are USERs: '0' after move makes a creen black
							 and second press '0' changes a player to another
after ending a placing ships it needs to press '0' like in the previous situation 

to autoplacing ships in USER mode comment lines 99-100 in file player.cpp
*/

//#include <iostream>
#include <windows.h>
#include <windowsx.h> // GetStockBrush
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>
#include "game.h"

// for draw
#define betweenMoveColor RGB(0, 0, 0)
#define backgroundColor RGB(255, 255, 255)
#define backgroundLineColor RGB(130, 235, 215)
#define boxLineColor RGB(111, 0, 255)
#define pointAreaColor RGB(231, 69, 96)
#define pointBorderColor RGB(231, 69, 96)
#define crossColor RGB(231, 69, 96)

typedef enum { FIRST_PLAYER_T, FIRST_OPPONENT_T, SECOND_PLAYER_T, SECOND_OPPONENT_T, NULL_T } TABLE_TO_DISPLAY;

struct Position {
	TABLE_TO_DISPLAY table;
	int xTablePos;
	int yTablePos;
};

const int SHIP_PEN_WIDTH = 3;
const int BOX_LINE_WIDTH = 2;
const int SROSS_LINE_WIDTH = 2;
const int FIELD_LINE_WIDTH = 1;
const int POINT_LINE_WIDTH = 1;
const int POINT_RADIUS = 5;

const int CELL_SIZE = 20;
const int X_CELLS = 25;
const int Y_CELLS = 28;
const int WIDTH = X_CELLS * CELL_SIZE;
const int HEIGHT = Y_CELLS * CELL_SIZE;

const std::pair<int, int> FIRST_PLAYER_TABLE_BEGIN = { 1, 2 };
const std::pair<int, int> FIRST_OPPONENT_TABLE_BEGIN = { 14, 2 };
const std::pair<int, int> SECOND_PLAYER_TABLE_BEGIN = { 1, 15 };
const std::pair<int, int> SECOND_OPPONENT_TABLE_BEGIN = { 14, 15 };

const UINT_PTR MOVE_TIMER = 1;
const UINT_PTR DRAW_TIMER = 2;
const UINT MOVE_TIMER_SPEED = 1000;
const UINT DRAW_TIMER_SPEED = 1;

game g = game(BOT, BOT);
bool CLEAN_VIEW = true;

std::vector<Position> shipPosFirst;
std::vector<Position> shipPosSecond;

bool toBlackScreen = false;
PLAYER_NUMBER activePlayer = FIRST_PLAYER;
UINT_PTR MoveTimerId, DrawTimerId;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"SEA_BATTLE";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);
	
	HWND hwnd1 = CreateWindowEx(
		0,                      
		CLASS_NAME,               
		L"SEA-BATTLE",  
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,     

		CW_USEDEFAULT, CW_USEDEFAULT, 
		WIDTH,
		HEIGHT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd1 == NULL) {
		return 0;
	}

	ShowWindow(hwnd1, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

bool rememberForPlace(int xWindowPos, int yWindowPos);
Position getPosition(int xWindowPos, int yWindowPos);
std::pair<int, int> getTablePos(TABLE_TO_DISPLAY table);
void drawLines(HDC hDC);
void drawText(HDC hDC, std::wstring wstr, int xPos, int yPos);
void drawArea(HDC hDC);
void drawCell(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table);
void drawPoint(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table);
void drawCross(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table);
void draw(HWND hwnd);
void drawBlack(HWND hwnd);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		
		case WM_CREATE:
			MoveTimerId = SetTimer(hwnd, MOVE_TIMER, MOVE_TIMER_SPEED, NULL);
			DrawTimerId = SetTimer(hwnd, DRAW_TIMER, DRAW_TIMER_SPEED, NULL);
			return 0;

		case WM_LBUTTONDOWN:
			{
			int xWindowPos = LOWORD(lParam) / CELL_SIZE;
			int yWindowPos = HIWORD(lParam) / CELL_SIZE;

			rememberForPlace(xWindowPos, yWindowPos);
			}
			return 0;

		
		case WM_KEYDOWN:
			switch (wParam) {
				case 96: /* 0 */
					if (toBlackScreen) {
						drawBlack(hwnd);
						toBlackScreen = false;
					}
					else {
						activePlayer = g.whoGoes();
						draw(hwnd);
					}
					break;
				case VK_RETURN:
					{
					std::vector<Position>* shipPos;
					char buffer[0xff];

					if (!g.firstIsReady())
						shipPos = &shipPosFirst;
					else if (!g.secondIsReady())
						shipPos = &shipPosSecond;
					else if (activePlayer == FIRST_PLAYER)
						shipPos = &shipPosFirst;
					else if (activePlayer == SECOND_PLAYER)
						shipPos = &shipPosSecond;
					else
						return 0;

					if (g.isReady()) {
						if (shipPos->size() == 0)
							return 0;

						Position pos = shipPos->back();
						shipPos->clear();
						switch (pos.table) {
							case FIRST_OPPONENT_T:
								if (g.move(FIRST_PLAYER, pos.xTablePos, pos.yTablePos)) {

									if (g.isFinish()) {
										sprintf_s(buffer, "win move: %c %d\n", (char)(65 + pos.xTablePos), pos.yTablePos + 1);
										MessageBoxA(NULL, buffer, "WIN", MB_OK);
										KillTimer(hwnd, MoveTimerId);
										KillTimer(hwnd, DrawTimerId);
										return 0;
									}

									if (g.whoGoes() == SECOND_PLAYER && g.getSecondType() == USER) {
										draw(hwnd);
										toBlackScreen = true;
										activePlayer = NULL_PLAYER;
									}
									activePlayer = (activePlayer == NULL_PLAYER) ? NULL_PLAYER : FIRST_PLAYER;

									sprintf_s(buffer, "move: %c %d\n", (char)(65 + pos.xTablePos), pos.yTablePos + 1);
									MessageBoxA(NULL, buffer, "MOVE", MB_OK);
									return 0;
								}
								break;
							case SECOND_OPPONENT_T:
								if (g.move(SECOND_PLAYER, pos.xTablePos, pos.yTablePos)) {

									if (g.isFinish()) {
										sprintf_s(buffer, "win move: %c %d\n", (char)(65 + pos.xTablePos), pos.yTablePos + 1);
										MessageBoxA(NULL, buffer, "WIN", MB_OK);
										KillTimer(hwnd, MoveTimerId);
										KillTimer(hwnd, DrawTimerId);
										return 0;
									}

									if (g.whoGoes() == FIRST_PLAYER && g.getFirstType() == USER) {
										draw(hwnd);
										toBlackScreen = true;
										activePlayer = NULL_PLAYER;
									}
									activePlayer = (activePlayer == NULL_PLAYER) ? NULL_PLAYER : SECOND_PLAYER;

									sprintf_s(buffer, "move: %c %d\n", (char)(65 + pos.xTablePos), pos.yTablePos + 1);
									MessageBoxA(NULL, buffer, "MOVE", MB_OK);
									return 0;
								}
								break;
						}
						return 0;
					}

					if (shipPos->size() != 2) {
						sprintf_s(buffer, "Count error: %d\n", shipPos->size());
						MessageBoxA(NULL, buffer, "ENTER", MB_OK);
						shipPos->clear();
						return 0;
					}
					Position posStart = (*shipPos)[0];
					Position posEnd = (*shipPos)[1];

					if (posStart.table == posEnd.table) {
						if (posStart.table == FIRST_PLAYER_T)
							g.setShipIfOk(FIRST_PLAYER, posStart.xTablePos, posStart.yTablePos, posEnd.xTablePos, posEnd.yTablePos);
						if (posStart.table == SECOND_PLAYER_T)
							g.setShipIfOk(SECOND_PLAYER, posStart.xTablePos, posStart.yTablePos, posEnd.xTablePos, posEnd.yTablePos);
					}

					if (g.firstIsReady()) {
						activePlayer = SECOND_PLAYER;
						if (g.secondIsReady()) {
							draw(hwnd);
							toBlackScreen = true;
							activePlayer = NULL_PLAYER;
						}
						draw(hwnd);
					}
						

					sprintf_s(buffer, "ship: (%c %d) (%c %d)\n", (char)(65 + posStart.xTablePos), posStart.yTablePos + 1, 
															     (char)(65 + posEnd.xTablePos), posEnd.yTablePos + 1);
					MessageBoxA(NULL, buffer, "ENTER", MB_OK);
					}
			}
			return 0;

		case WM_TIMER:
			switch (wParam) {
				case MOVE_TIMER:
					if (toBlackScreen)
						return 0;

					if (g.move()) {
						/*char buffer[0xff];
						for (int i = 0; i < g.positions.size(); ++i) {
							sprintf_s(buffer, "comp move: %d %d\n", g.positions[i].first, g.positions[i].second);
							MessageBoxA(NULL, buffer, "MOVE", MB_OK);
						}*/
						activePlayer = g.whoGoes();
					}

					return 0;
				case DRAW_TIMER:
					draw(hwnd);
					return 0;
				default:
					MessageBox(hwnd, TEXT("?????"), TEXT("TIMER_ERROR"), MB_OK);
					return 0;
			}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool rememberForPlace(int xWindowPos, int yWindowPos) {
	char buffer[0xff];
	std::vector<Position>* shipPos = NULL;
	
	if (activePlayer == NULL_PLAYER || g.isFinish())
		return false;

	Position pos = getPosition(xWindowPos, yWindowPos);

	if (pos.table == FIRST_PLAYER_T && !g.firstIsReady() && g.getFirstType() == USER)
		shipPos = &shipPosFirst;
	else if (pos.table == FIRST_OPPONENT_T && g.firstIsReady() && activePlayer == FIRST_PLAYER && g.getFirstType() == USER)
		shipPos = &shipPosFirst;
	else if (pos.table == SECOND_PLAYER_T && g.firstIsReady() && !g.secondIsReady() && g.getSecondType() == USER)
		shipPos = &shipPosSecond;
	else if (pos.table == SECOND_OPPONENT_T && g.secondIsReady() && activePlayer == SECOND_PLAYER && g.getSecondType() == USER)
		shipPos = &shipPosSecond;
	else
		return false;

	if (shipPos->size() == 2)
		shipPos->clear();

	shipPos->push_back(pos);

	sprintf_s(buffer, "Selected: %c %d\n", (char)(65 + pos.xTablePos), pos.yTablePos + 1);

	if (g.isReady()) {
		MessageBoxA(NULL, buffer, "TO SHOOT", MB_OK);
		return true;
	}

	if (shipPos->size() == 1) 
		MessageBoxA(NULL, buffer, "START", MB_OK);
	else
		MessageBoxA(NULL, buffer, "END", MB_OK);
	
	return true;
}

Position getPosition(int xWindowPos, int yWindowPos) {
	Position pos;
	
	pos.table = NULL_T;
	if (yWindowPos >= 2 && yWindowPos <= 2 + TABLE_SIZE - 1) {
		if (xWindowPos >= 1 && xWindowPos <= 1 + TABLE_SIZE - 1) {
			pos.table = FIRST_PLAYER_T;
			pos.xTablePos = xWindowPos - 1;
			pos.yTablePos = yWindowPos - 2;
		}
		if (xWindowPos >= 14 && xWindowPos <= 14 + TABLE_SIZE - 1) {
			pos.table = FIRST_OPPONENT_T;
			pos.xTablePos = xWindowPos - 14;
			pos.yTablePos = yWindowPos - 2;
		}
	}
	if (yWindowPos >= 15 && yWindowPos <= 15 + TABLE_SIZE - 1) { 
		if (xWindowPos >= 1 && xWindowPos <= 1 + TABLE_SIZE - 1) {
			pos.table = SECOND_PLAYER_T;
			pos.xTablePos = xWindowPos - 1;
			pos.yTablePos = yWindowPos - 15;
		}
		if (xWindowPos >= 14 && xWindowPos <= 14 + TABLE_SIZE - 1) {
			pos.table = SECOND_OPPONENT_T;
			pos.xTablePos = xWindowPos - 14;
			pos.yTablePos = yWindowPos - 15;
		}
	}
	return pos;
}

std::pair<int, int> getTablePos(TABLE_TO_DISPLAY table) {
	switch (table) {
		case FIRST_PLAYER_T:
			return FIRST_PLAYER_TABLE_BEGIN;
		case FIRST_OPPONENT_T:
			return FIRST_OPPONENT_TABLE_BEGIN;
		case SECOND_PLAYER_T:
			return SECOND_PLAYER_TABLE_BEGIN;
		case SECOND_OPPONENT_T:
			return SECOND_OPPONENT_TABLE_BEGIN;
	}
}

void drawBlack(HWND hwnd) {
	HDC hDC = GetDC(hwnd);
	HBRUSH hBrush = CreateSolidBrush(betweenMoveColor);
	HBRUSH hOldBrush = (HBRUSH)(SelectObject(hDC, hBrush));

	Rectangle(hDC, 0, 0, WIDTH, HEIGHT);

	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);
	ReleaseDC(hwnd, hDC);
}

void draw(HWND hwnd) {
	HDC hDC = GetDC(hwnd);
	HDC hMemoryDC = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, WIDTH, HEIGHT);
	HBITMAP hOldBitmap = (HBITMAP) (SelectObject(hMemoryDC, hBitmap));
	PatBlt(hMemoryDC, 0, 0, WIDTH, HEIGHT, WHITENESS);

	if (activePlayer == NULL_PLAYER)
		return;

	drawArea(hMemoryDC);


	if (activePlayer == FIRST_PLAYER || (g.secondType_ == BOT) || CLEAN_VIEW) {
		if ((g.firstType_ == USER) || CLEAN_VIEW) {
			for (int xPos = 0; xPos < TABLE_SIZE; ++xPos) {
				for (int yPos = 0; yPos < TABLE_SIZE; ++yPos) {
					if (g.first_.getStatus(xPos, yPos, PLAYER) == SHIP || g.first_.getStatus(xPos, yPos, PLAYER) == HIT)
						drawCell(hMemoryDC, xPos + 1, yPos + 1, FIRST_PLAYER_T);

					if (g.second_.getStatus(xPos, yPos, OPPONENT) == SHIP_AREA || g.second_.getStatus(xPos, yPos, OPPONENT) == MISS)
						drawPoint(hMemoryDC, xPos + 1, yPos + 1, FIRST_PLAYER_T);
					if (g.second_.getStatus(xPos, yPos, OPPONENT) == HIT)
						drawCross(hMemoryDC, xPos + 1, yPos + 1, FIRST_PLAYER_T);

					if (g.first_.getStatus(xPos, yPos, OPPONENT) == SHIP_AREA || g.first_.getStatus(xPos, yPos, OPPONENT) == MISS)
						drawPoint(hMemoryDC, xPos + 1, yPos + 1, FIRST_OPPONENT_T);
					if (g.first_.getStatus(xPos, yPos, OPPONENT) == HIT)
						drawCross(hMemoryDC, xPos + 1, yPos + 1, FIRST_OPPONENT_T);
				}
			}
		}
	}

	if (activePlayer == SECOND_PLAYER || (g.firstType_ == BOT) || CLEAN_VIEW) {
		if ((g.secondType_ == USER) || CLEAN_VIEW) {
			for (int xPos = 0; xPos < TABLE_SIZE; ++xPos) {
				for (int yPos = 0; yPos < TABLE_SIZE; ++yPos) {
					if (g.second_.getStatus(xPos, yPos, PLAYER) == SHIP || g.second_.getStatus(xPos, yPos, PLAYER) == HIT)
						drawCell(hMemoryDC, xPos + 1, yPos + 1, SECOND_PLAYER_T);

					if (g.first_.getStatus(xPos, yPos, OPPONENT) == SHIP_AREA || g.first_.getStatus(xPos, yPos, OPPONENT) == MISS)
						drawPoint(hMemoryDC, xPos + 1, yPos + 1, SECOND_PLAYER_T);
					if (g.first_.getStatus(xPos, yPos, OPPONENT) == HIT)
						drawCross(hMemoryDC, xPos + 1, yPos + 1, SECOND_PLAYER_T);

					if (g.second_.getStatus(xPos, yPos, OPPONENT) == SHIP_AREA || g.second_.getStatus(xPos, yPos, OPPONENT) == MISS)
						drawPoint(hMemoryDC, xPos + 1, yPos + 1, SECOND_OPPONENT_T);
					if (g.second_.getStatus(xPos, yPos, OPPONENT) == HIT)
						drawCross(hMemoryDC, xPos + 1, yPos + 1, SECOND_OPPONENT_T);
				}
			}
		}
	}

	BitBlt(hDC, 0, 0, WIDTH, HEIGHT, hMemoryDC, 0, 0, SRCCOPY);
	SelectObject(hMemoryDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	ReleaseDC(hwnd, hDC);
}

void drawText(HDC hDC, std::wstring wstr, int xPos, int yPos) {
	SetBkMode(hDC, TRANSPARENT);
	TextOut(hDC, xPos, yPos, wstr.c_str(), (int)(wstr.length()));
}

void drawCell(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table) {
	HBRUSH hBrush = CreateSolidBrush(backgroundColor);
	HPEN hPen = CreatePen(PS_SOLID, SHIP_PEN_WIDTH, boxLineColor);
	HBRUSH hOldBrush = (HBRUSH) (SelectObject(hDC, hBrush));
	HPEN hOldPen = (HPEN) (SelectObject(hDC, hPen));

	std::pair<int, int> tablePos = getTablePos(table);
	Rectangle(hDC, (tablePos.first - 1 + xStartPos) * CELL_SIZE, (tablePos.second - 1 + yStartPos) * CELL_SIZE, 
				   (tablePos.first + xStartPos) * CELL_SIZE + 1, (tablePos.second + yStartPos) * CELL_SIZE + 1);

	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

void drawPoint(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table) {
	HPEN hPen = CreatePen(PS_SOLID, POINT_LINE_WIDTH, pointBorderColor);
	HPEN hOldPen = static_cast<HPEN>(SelectObject(hDC, hPen));
	HBRUSH hBrush = CreateSolidBrush(pointAreaColor);
	HBRUSH hOldBrush = (HBRUSH)(SelectObject(hDC, hBrush));

	std::pair<int, int> tablePos = getTablePos(table);

	Ellipse(hDC, (int) (CELL_SIZE * (tablePos.first + xStartPos - 0.5) - POINT_RADIUS), (int) (CELL_SIZE * (tablePos.second + yStartPos - 0.5) - POINT_RADIUS),
				 (int) (CELL_SIZE * (tablePos.first + xStartPos - 0.5) + POINT_RADIUS), (int) (CELL_SIZE * (tablePos.second + yStartPos - 0.5) + POINT_RADIUS));

	DeleteObject(hBrush);
	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}

void drawCross(HDC hDC, int xStartPos, int yStartPos, TABLE_TO_DISPLAY table) {
	HPEN hPen = CreatePen(PS_SOLID, SROSS_LINE_WIDTH, crossColor);
	HPEN hOldPen = (HPEN) (SelectObject(hDC, hPen));

	std::pair<int, int> tablePos = getTablePos(table);

	MoveToEx(hDC, (tablePos.first - 1 + xStartPos) * CELL_SIZE + 2, (tablePos.second - 1 + yStartPos) * CELL_SIZE + 2, NULL);
	LineTo(hDC, (tablePos.first + xStartPos) * CELL_SIZE - 2, (tablePos.second + yStartPos) * CELL_SIZE - 2);
	MoveToEx(hDC, (tablePos.first - 1 + xStartPos) * CELL_SIZE + 2, (tablePos.second + yStartPos) * CELL_SIZE - 2, NULL);
	LineTo(hDC, (tablePos.first + xStartPos) * CELL_SIZE - 2, (tablePos.second - 1 + yStartPos) * CELL_SIZE + 2);

	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}

void drawArea(HDC hDC) {
	drawLines(hDC);

	HPEN hPen = CreatePen(PS_SOLID, BOX_LINE_WIDTH, boxLineColor);
	HPEN hOldPen = (HPEN) (SelectObject(hDC, hPen));
	HBRUSH hOldBrush = (HBRUSH) (SelectObject(hDC, GetStockBrush(NULL_BRUSH)));

	std::pair<int, int> tablesPointer[4] = { FIRST_PLAYER_TABLE_BEGIN, FIRST_OPPONENT_TABLE_BEGIN, SECOND_PLAYER_TABLE_BEGIN, SECOND_OPPONENT_TABLE_BEGIN };

	drawText(hDC, std::to_wstring(g.getShoots(FIRST_PLAYER)), CELL_SIZE + 6, 3);
	drawText(hDC, std::wstring(1, (char)(58)), CELL_SIZE * 2 + 10, 3);
	drawText(hDC, std::to_wstring(g.getShoots(SECOND_PLAYER)), CELL_SIZE * 3 + 6, 3);

	for (int table_num = 0; table_num < 4; ++table_num) {
		Rectangle(hDC, CELL_SIZE * tablesPointer[table_num].first, CELL_SIZE * tablesPointer[table_num].second,
			CELL_SIZE * (tablesPointer[table_num].first + TABLE_SIZE) + 1, CELL_SIZE * (tablesPointer[table_num].second + TABLE_SIZE) + 1);

		for (int number = 1; number <= TABLE_SIZE; ++number) {
			drawText(hDC, std::wstring(1, (char)(64 + number)), CELL_SIZE * (tablesPointer[table_num].first - 1 + number) + 6,
				CELL_SIZE * (tablesPointer[table_num].second - 1) + 2);
			
			drawText(hDC, std::to_wstring(number), CELL_SIZE * (tablesPointer[table_num].first - 1) + 2,
				CELL_SIZE * (tablesPointer[table_num].second - 1 + number) + 2);
		}
	}

	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}

void drawLines(HDC hDC) {
	HBRUSH hBrush = CreateSolidBrush(backgroundColor);
	HBRUSH hOldBrush = (HBRUSH) (SelectObject(hDC, hBrush));

	Rectangle(hDC, 0, 0, WIDTH, HEIGHT);

	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);

	HPEN hPen = CreatePen(PS_SOLID, FIELD_LINE_WIDTH, backgroundLineColor);
	HPEN hOldPen = (HPEN) (SelectObject(hDC, hPen));

	for (int xCell = 0; xCell < X_CELLS; ++xCell) {
		MoveToEx(hDC, xCell*CELL_SIZE, 0, NULL);
		LineTo(hDC, xCell*CELL_SIZE, HEIGHT);
	}

	for (int yCell = 0; yCell < Y_CELLS; ++yCell) {
		MoveToEx(hDC, 0, yCell*CELL_SIZE, NULL);
		LineTo(hDC, WIDTH, yCell*CELL_SIZE);
	}

	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
}