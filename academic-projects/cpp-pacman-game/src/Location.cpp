/*********************************************************
*
* Description: Board coordinate constructor and factory implementations.
* Author: Moshe Yifrah
*
*********************************************************/

#include "Location.h"

Location::Location(int row, int col) : row(row), col(col) {}
Location::Location() : row(0) , col(0) {}

Location enterLocation(int m_row, int m_col)
{
	return { m_row , m_col };
}