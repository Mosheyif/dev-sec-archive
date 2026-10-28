/*********************************************************
*
* Description: Board coordinate type for row/column positions.
* Author: Moshe Yifrah
*
*********************************************************/

#pragma once

class Location
{
public:
    Location(int row, int col);

   Location();

    int row ;
    int col ;

    Location enterLocation(int, int);
};
