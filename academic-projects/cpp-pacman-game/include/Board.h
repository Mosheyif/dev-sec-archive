/*********************************************************
*
* Description: Game board state — tile grid, entity locations, and level reset.
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
class Board {
public:

	Board();
	void resetBoard();

	void getLocation(std::vector <Location>& , char , int);

	int countItem(char lookForChar);

	void printBoard();

	char getChar(Location& location);


private:
std::ifstream m_file;
std::vector <std::string> m_board;
std::vector <Location> m_location;
//std::string m_line;
int m_sizeRow;
int m_sizeCol;
int m_lives = 3;
int m_score = 3;
Location location;

};

