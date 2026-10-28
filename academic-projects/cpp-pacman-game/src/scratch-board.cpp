//#include "Board.h"
//#include <vector>
//#include <iostream>
//#include <string>
//#include <fstream>
//
//#include "Board.h"
//#include "Location.h"
//int main() {
//
//
//	//=========================================================
//	//class Board {
//	//public:
//	//	Board(int lives, int score);
//		//void getLocation(char lookForChar, Location& arr[]);
//
//
//
////	private:
//		std::ifstream m_file;
//		std::vector <std::string> m_board;
//		std::string m_line;
//		int m_sizeRow = 5;
//		int m_sizeCol = 5;
//		int m_lives  = 5;
//		int m_score = 5;
//		
////	};
//
//	//Board::Board(int lives, int score)
//	//	: m_lives(lives), m_score(score)
//			
//
//			if (!m_file.is_open())
//			 m_file.open("Board.txt");
//			
//
//			 for (int i = 0; i < 3; i++){
//			 m_board.clear();
//				m_file >> m_line;
//				m_sizeRow = { std::stoi(m_line) };
//			
//				m_file >> m_line;
//				m_sizeCol = { std::stoi(m_line) };
//			
//				while (!m_file.eof()) {
//			
//					getline(m_file, m_line);
//					m_board.push_back(m_line);
//						if (m_line.empty())
//							break;
//				}
//					for (auto file_line : m_board)
//						std::cout << file_line << std::endl;
//
//
//					std::cout << "Lives left: " << m_lives << std::endl <<
//						"Your score: " << m_score << std::endl;
//
//				}
//
//	void Board::getLocation(char lookForChar, Location  arr[])
//
//		int i = 0;
//		for (int row = 0; row < m_sizeRow; ++row)
//			for (int col = 0; col < m_sizeCol; ++col)
//				if (lookForChar == m_board[row][col])
//				{
//					arr[i].row = row;
//					arr[i].col = col;
//					i++;
//				}
//	}
//
//
//
//	return 0;
//}