/********************************************************************
CSCI 490J - Assignment 4 - Spring 2026
Programmer: Jacob Yates
Last Updated: 4/18/2026

Purpose: This header implements a simple attributed point
        structure that contains lots of important data for our
        polygon and lighting pipelines.
*********************************************************************/

#include "rd_pointa.h"
#include "rd_pointc.h"

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

/// Copy constructor that returns a new point with the
/// same values as the current point.
rd_pointa::rd_pointa(const rd_pointa &point)
{
    for (int index = 0; index < ATTR_SIZE; index++)
        coord[index] = point.coord[index];
}

/// Creates a new attributed point with the
/// xyz values from the cartesian point and w
/// set to 1.
rd_pointa::rd_pointa(const rd_pointc &point)
{
    coord[0] = point.get_x();
    coord[1] = point.get_y();
    coord[2] = point.get_z();
    coord[3] = 1;

    // Fill the rest in with 0s
    for (int index = 4; index < ATTR_SIZE; index++)
        coord[index] = 0;
}

/// @returns This attributed point with all values in coord
///     increased by the values in point.coord.
rd_pointa rd_pointa::operator+(rd_pointa point)
{
    rd_pointa result;
    for (int index = 0; index < ATTR_SIZE; index++)
        result.coord[index] = coord[index] + point.coord[index];

    return result;
}

/// @returns This attributed point with all values in coord
///     subtracted by the values in point.coord.
rd_pointa rd_pointa::operator-(rd_pointa point)
{
    rd_pointa result;
    for (int index = 0; index < ATTR_SIZE; index++)
        result.coord[index] = coord[index] - point.coord[index];

    return result;
}

/// @returns This attributed point with all values in coord
///     divided by the divisor.
rd_pointa rd_pointa::operator/(float divisor)
{
    rd_pointa result;
    for (int index = 0; index < ATTR_SIZE; index++)
        result.coord[index] = coord[index] / divisor;

    return result;
}

/// @returns This attributed point with all values in coord
///     multiplied by the factor. This implements left-hand
///     multiplication :)
rd_pointa operator*(float factor, const rd_pointa& point)
{
    rd_pointa result;
    for (int index = 0; index < ATTR_SIZE; index++)
        result.coord[index] = point.coord[index] * factor;

    return result;
}

/// Utility function to more easily enter the normal vector values
/// into the attributed point.
/// @param nx The normal x-direction.
/// @param ny The normal y-direction.
/// @param nz The normal z-direction.
void rd_pointa::set_normal(float nx, float ny, float nz)
{
    coord[ATTR_NX] = nx;
    coord[ATTR_NY] = ny;
    coord[ATTR_NZ] = nz;
}