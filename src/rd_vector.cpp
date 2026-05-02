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
#include <iostream>
#include "rd_pointc.h"

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

/// Constructor that creates a vector from the components of the point provided.
rd_vector::rd_vector(rd_pointc point)
{
    this->x = point.get_x();
    this->y = point.get_y();
    this->z = point.get_z();
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

/// @param point The cartesian point to dot together with this one.
/// @returns The dot product scalar of the vector and the point.
float rd_vector::operator^(class rd_pointc point)
{
    return (x * point.get_x() + y * point.get_y() + z * point.get_z());
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

/// Allows for left-hand multiplication of a vector for ease of
/// writing formulas without having to worry about the side of multiplication.
/// @returns The vector on the right multiplied by the float on the left.
rd_vector operator* (float factor, const rd_vector& vector)
{
    return rd_vector(vector.GetX() * factor, vector.GetY() * factor, vector.GetZ() * factor);
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

/// @param index The index of the data to access.
/// @returns The value at the index inside the vector.
float& rd_vector::operator[](int index)
{
    switch (index)
    {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::out_of_range("Index cannot exceed 3 on a homogenous point.");
    }
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

/// @returns A vector pointing in the opposite direction
/// of the current vector.
rd_vector rd_vector::flipped_vector()
{
    return rd_vector(-x, -y, -z);
}

/// @param v1 The vector to reflect upon, typically a surface normal.
/// @param v2 The vector being reflected about v1, typically a light vector.
/// @returns The reflection of v2 across v1.
rd_vector rd_vector::reflect(rd_vector light, rd_vector normal)
{
    //return light - 2 * (normal ^ light) / powf(normal.magnitude(), 2) * normal;
    return (2 * (normal ^ light) / powf(normal.magnitude(), 2) * normal) - light;
}

/// Prints the contents of the vector to standard output for debugging purposes.
void rd_vector::print_vector()
{
    std::cout << "X: " << x << " Y: " << y << " Z: " << z << std::endl;
}
