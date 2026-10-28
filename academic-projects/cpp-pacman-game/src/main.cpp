/*********************************************************
*
* Description: Entry point — initializes and runs the Pacman game controller.
* Author: Moshe Yifrah
*
*********************************************************/

#include <iostream>
#include <cstdlib> // for std::system()
#include "io.h"
#include "player.h"
#include <vector>
#include <conio.h>
#include "Controller.h"
#include"Board.h"

int main()
{
    auto c = Controller();
    c.run();
}
