/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a mathematical vector as well as a
        vector addition, subtractions, dot product, and cross
        multiplication. It also has a utility to quickly find the
        magnitude.
*********************************************************************/

#include "rd_vector.h"
#include <cmath>

/// Default constructor that initializes all components to 0.
rd_vector::rd_vector()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

/// Constructor that initializes all components to the provided values.
rd_vector::rd_vector(float x, float y, float z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

/// @param v The vector being added to this one.
/// @returns A new vector with the components of each vector added together.
rd_vector rd_vector::operator+ (rd_vector v)
{
    return rd_vector(x + v.x, y + v.y, z + v.z);
}

/// @param v The vector being subtracted from this one.
/// @returns A new vector with the components of v subtracted from the components of this vector.
rd_vector rd_vector::operator- (rd_vector v)
{
    return rd_vector(x - v.x, y - v.y, z - v.z);
}

/// @param v The vector to dot together with this one.
/// @returns The dot product scalar of the two vectors.
float rd_vector::operator^ (rd_vector v)
{
    return ((x * v.x) + (y * v.y) + (z * v.z));
}

/// @param v The vector to cross with this one.
/// @returns The cross product of the two vectors.
rd_vector rd_vector::operator* (rd_vector v)
{
    float newX = y * v.z - z * v.y;
    float newY = z * v.x - x * v.z;
    float newZ = x * v.y - y * v.x;
    return rd_vector(newX, newY, newZ);
}

/// @param scalar The scalar to multiply each component by.
/// @returns The scaled vector.
rd_vector rd_vector::operator* (float scalar)
{
    return rd_vector(x * scalar, y * scalar, z * scalar);
}

/// @param v The vector to copy onto this one.
/// @returns A reference to this object which has been updated.
rd_vector& rd_vector::operator= (const rd_vector &v)
{
    // Set our values to the ones from v
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;

    return *this;
}

/// @returns The magnitude of the vector.
float rd_vector::magnitude()
{
    return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
}

/// @returns The normalized version of this vector.
rd_vector rd_vector::normalized()
{
    float magnitude = this->magnitude();
    return rd_vector(x / magnitude, y / magnitude, z / magnitude);
}
