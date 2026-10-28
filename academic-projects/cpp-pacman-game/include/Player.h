/*********************************************************
*
* Description: Player entity — input handling, movement, and board collision.
* Author: Moshe Yifrah
*
*********************************************************/

#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <stdio.h>
#include"Location.h"
#include"Demon.h"
#include<stdlib.h>
#include"Board.h"
#include"io.h"

enum Keys
{
	KB_Escape = 27,
	SpecialKey = 224,
};

enum SpecialKeys
{
	KB_Up = 72,
	KB_Down = 80,
	KB_Left = 75,
	KB_Right = 77,
};

class Player {

public:

	void setLocation(std::vector <Location>);

	Location getLocation();

	Location getOrder();

	Location handleRegularKey(int c);

	Location handleSpecialKey();

	void move(Board& m_board, int& cokies);

	bool isLegal(Board& board, Location& m_move, int& cokies);

	void printMove(Location& prev, Location& current);


private:
	Location m_packman , m_start , m_move;
};
