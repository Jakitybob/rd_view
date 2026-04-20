/********************************************************************
CSCI 490J - Assignment 4 - Spring 2026
Programmer: Jacob Yates
Last Updated: 4/18/2026

Purpose: This header implements a simple attributed point
        structure that contains lots of important data for our
        polygon and lighting pipelines.
*********************************************************************/

#include "rd_pointa.h"

/// Default constructor to fill the array with 0 so it has
/// valid values no matter what.
rd_pointa::rd_pointa()
{
    for (int index = 0; index < ATTR_SIZE; index++)
        coord[index] = 0;
}

/// Constructs an attributed point with indices 0-4 having the
/// provided xyzw values.
rd_pointa::rd_pointa(float x, float y, float z, float w)
{
    coord[0] = x;
    coord[1] = y;
    coord[2] = z;
    coord[3] = w;

    // Fill the rest in with 0s
    for (int index = 4; index < ATTR_SIZE; index++)
        coord[index] = 0;
}

/// @returns This attributed point with all values in coord
///     increased by the values in point.coord.
rd_pointa rd_pointa::operator+(rd_pointa point)
{
    for (int index = 0; index < ATTR_SIZE; index++)
        coord[index] = coord[index] + point.coord[index];

    return *this;
}

/// @returns This attributed point with all values in coord
///     subtracted by the values in point.coord.
rd_pointa rd_pointa::operator-(rd_pointa point)
{
    for (int index = 0; index < ATTR_SIZE; index++)
        coord[index] = coord[index] - point.coord[index];

    return *this;
}

/// @returns This attributed point with all values in coord
///     divided by the divisor.
rd_pointa rd_pointa::operator/(float divisor)
{
    for (int index = 0; index < ATTR_SIZE; index++)
        coord[index] = coord[index] / divisor;

    return *this;
}

/// @returns This attributed point with all values in coord
///     multiplied by the factor. This implements left-hand
///     multiplication :)
rd_pointa operator*(float factor, const rd_pointa& point)
{
    rd_pointa new_point;
    for (int index = 0; index < ATTR_SIZE; index++)
        new_point.coord[index] = point.coord[index] * factor;

    return new_point;
}