/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a simple 3D point with a number
        of operator overloads to allow easier use of points in the
        RD View application.
*********************************************************************/

#include "rd_point.h"
#include "rd_vector.h"
#include "rd_pointh.h"

/// @returns A point initialized to the origin (0, 0, 0).
rd_point::rd_point()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

/// @returns A point initialized to the provided XYZ values.
rd_point::rd_point(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

/// @returns A point initialized to the XYZ values in the float array passed in.
rd_point::rd_point(const float *point)
{
    this->x = point[0];
    this->y = point[1];
    this->z = point[2];
}

/// @returns A point converted from the provided homogenous point.
rd_point::rd_point(class rd_pointh point)
{
    this->x = point.get_x() / point.get_w();
    this->y = point.get_y() / point.get_w();
    this->z = point.get_z() / point.get_w();
}

/// @returns The vector found from subtracting our points together.
rd_vector rd_point::operator-(rd_point point)
{
    return rd_vector(point.x - x, point.y - y, point.z - z);
}

/// @returns The point scaled by the scalar.
rd_point rd_point::operator*(float scalar)
{
    return {x * scalar, y * scalar, z * scalar};
}

/// @returns A reference to this class with the updated values.
rd_point& rd_point::operator=(rd_point point)
{
    this->x = point.x;
    this->y = point.y;
    this->z = point.z;

    return *this;
}
