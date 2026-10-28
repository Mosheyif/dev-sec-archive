# *********************************************************
#
# Description: Calculations of areas of a circle, triangle
# and a rectangle module
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from math import pi

# --- functions ---

def get_circle_area(radius):
    """Get the area of a circle."""
    return pi * radius ** 2


def get_triangle_area(height, width):
    """Get the area of a triangle."""
    return height * width / 2


def get_rectangle_area(height, width):
    """Get the area of a rectangle."""
    return height * width
