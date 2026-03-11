/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a simple 3D homogenous point with a number
        of operator overloads to allow easier use of points in the
        RD View application.
*********************************************************************/

#include "rd_pointh.h"
#include <stdexcept>
#include "rd_point.h"

/// @returns A homogenous point initialized to the origin.
rd_pointh::rd_pointh()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
    this->w = 1;
}

rd_pointh::rd_pointh(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = 1;
}

rd_pointh::rd_pointh(const float *point)
{
    this->x = point[0];
    this->y = point[1];
    this->z = point[2];
    this->w = 1;
}

rd_pointh::rd_pointh(rd_point point)
{
    this->x = point.get_x();
    this->y = point.get_y();
    this->z = point.get_z();
    this->w = 1;
}

rd_pointh::rd_pointh(float x, float y, float z, float w)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

float& rd_pointh::operator[](int index)
{
    switch (index)
    {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            throw std::out_of_range("Index cannot exceed 3 on a homogenous point.");
    }
}
