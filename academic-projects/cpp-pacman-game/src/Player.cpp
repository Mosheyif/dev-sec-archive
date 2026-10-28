/*********************************************************
*
* Description: Player movement, input handling, and board interaction.
* Author: Moshe Yifrah
*
*********************************************************/

#include "Player.h"

void Player:: setLocation(std::vector <Location> location)
{
	m_packman = m_start = location[0];
}
/*********************************************************/

Location Player::getLocation()
{
    return m_packman;
}
/*********************************************************/

void Player::move(Board& board, int& cokies){

    Location prevPackman = m_packman;
    Location move = {0,0};

    while (move.row != 3) {

        move = getOrder();

        move.row += m_packman.row;
        move.col += m_packman.col;

        if (isLegal(board, move, cokies)) {
            m_packman.row = move.row;
            m_packman.col = move.col;
            break;
        }
    }

    printMove(prevPackman, m_packman);
}
/*********************************************************/

 Location Player::getOrder()
   {
         auto c = _getch();
         switch (c)
         {
         case 0:
         case SpecialKey: return (handleSpecialKey());
         default: return handleRegularKey(c);
         }

         return { 0,0 };
   }
/*********************************************************/

 Location Player::handleRegularKey(int c)
     {
         switch (c)
         {
         case ' ': return{ 0,0 };

         default: return { 3,0 };
         }
         return { 0,0 };
 }
/*********************************************************/

 Location Player::handleSpecialKey()
     {
         auto c = _getch();
         switch (c)
         {
         case KB_Up: return { -1,0};

         case KB_Down: return { 1,0 };

         case KB_Left: return{ 0,-1 };

         case KB_Right: return{ 0, 1 };

         default: return {3, 0};
         }
         return { 0,0 };
 }
/*********************************************************/

 bool Player::isLegal(Board& board , Location& move, int& cokies) {

     char c = board.getChar(move);

     if (c == '#')
         return false;

     if (c == '*')
         cokies--;

     return true;
}
/*********************************************************/

 void Player::printMove(Location& prev, Location& current) {

     Screen::setLocation(prev);

     std::cout << ' ';

     Screen::setLocation(current);

     std::cout << 'a';

 }
