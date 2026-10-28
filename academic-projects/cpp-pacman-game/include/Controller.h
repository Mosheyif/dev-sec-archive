/*********************************************************
*
* Description: Game controller — level loop, entity tracking, and win condition.
* Author: Moshe Yifrah
*
*********************************************************/

#pragma once
#include "Board.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include "Player.h"

const int NUMS_OF_LEVELS = 3;


class Controller{


public:

   Controller();

   void run();

   void sumItem();

 //  Board getLocation(char lookForChar, Location arr[]);


private:
	Board m_board;
	int m_lives = 0;
	int m_score = 0;
	int m_cockies = 0;
	int m_doors = 0;
	int m_wall = 0;
	int m_key = 0;
	int m_present = 0;
	int m_demons = 0;
	std::vector <Location> packmanLocation;
	Player m_player;



};