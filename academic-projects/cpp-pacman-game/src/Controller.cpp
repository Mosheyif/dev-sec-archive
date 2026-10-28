/*********************************************************
*
* Description: Game controller implementation — level loop and item counting.
* Author: Moshe Yifrah
*
*********************************************************/

#include "Controller.h"


Controller::Controller()
	: m_board()/*, m_player(m_board.getLocation('a'))*/
{

}
void Controller::run()
{

	   for (int i = 0; i < NUMS_OF_LEVELS; i++) {
		
		m_board.printBoard();

	    m_board.getLocation(packmanLocation , 'a' , 1);

		m_player.setLocation(packmanLocation);

		sumItem();

		while (m_cockies > 0) {

			m_player.move(m_board, m_cockies);



		}


		

		m_board.resetBoard();
		std::system("cls");

	}
}
/*********************************************************/

void Controller::sumItem() {

	m_demons = m_board.countItem('&');
	m_cockies = m_board.countItem('*');
	m_doors = m_board.countItem('D');
	m_wall = m_board.countItem('#');
	m_key = m_board.countItem('%');
	m_present = m_board.countItem('$');
}

