/*********************************************************
*
* Description: Game board implementation — loading, rendering, and entity lookup.
* Author: Moshe Yifrah
*
*********************************************************/

#include "Board.h"


Board::Board()
{
	if (!m_file.is_open())
		m_file.open("Board.txt");

	if (m_file.fail())
		std::cerr << "Error";

	resetBoard();
}
/*********************************************************/

void Board::resetBoard() {

	std::string line;

	m_board.clear();

	getline(m_file, line);
	auto iss = std::istringstream(line);

	iss >> m_sizeRow;
	iss >> m_sizeCol;
	
	while (!m_file.eof()) {

		getline(m_file, line);
		m_board.push_back(line);

		if (line.empty()) 
			break;
	}
}

/*********************************************************/

void Board::getLocation(std::vector <Location> &m_location , char lookForChar, int amount)
{
	int i = 0;
	m_location.resize(amount);

	for (int row = 0; row < m_sizeRow; ++row)
		for (int col = 0; col < m_sizeCol; ++col)
			if (lookForChar == m_board[row][col])
			{
				m_location[i].enterLocation(row, col);
				i++;
				if (i == amount)
					break;
			}
}
/*********************************************************/

 int Board::countItem(char lookForChar)
{
	int count = 0;
	for (int row = 0; row < m_sizeRow; ++row)
		for (int col = 0; col < m_sizeCol; ++col)
			if (lookForChar == m_board[row][col])
				count++;

	return count;
}
/*********************************************************/

 void Board::printBoard() {

	 for (auto file_line : m_board)
		 std::cout << file_line << std::endl;

	 std::cout << "Lives left: " << m_lives << std::endl <<
		 "Your score: " << m_score << std::endl;
 }
/*********************************************************/

 char Board::getChar(Location& location) {

	 return m_board[location.row][location.col];
}
