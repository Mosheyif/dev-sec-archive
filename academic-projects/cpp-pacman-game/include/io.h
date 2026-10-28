/*********************************************************
*
* Description: Terminal I/O utilities for cursor positioning.
* Author: Moshe Yifrah
*
*********************************************************/

#pragma once

#include "Location.h"

namespace Screen
{
// Reset terminal cursor location to start of the screen (0,0)
void resetLocation();
// Set terminal cursor location to the desired location
void setLocation(const Location& location);
}
