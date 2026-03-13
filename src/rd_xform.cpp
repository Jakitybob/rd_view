/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a row major 4x4 matrix used in RDView for transform
        matrices like translation, rotation, and scaling. It contains
        operations to multiply matrices together with one another and
        with homogenous points.
*********************************************************************/

#include "rd_xform.h"
#include "rd_pointh.h"
#include <cmath>
#include <iostream>

/// @returns A matrix initialized to the default identity matrix.
rd_xform::rd_xform()
{
    // Set the matrix to an identity matrix
    set_identity();
}

rd_xform::rd_xform(std::initializer_list<float> values)
{
    if (values.size() != 16)
        throw std::runtime_error("Transform matrices need 16 values.");

    auto iterator = values.begin();
    for (int rows = 0; rows < 4; rows++)
        for (int cols = 0; cols < 4; cols++)
            matrix[rows][cols] = *iterator++;
}

/// @param m2 The matrix being multiplied with this matrix.
/// @returns The new matrix found by multiplying the two matrices together.
rd_xform rd_xform::operator*(rd_xform m2)
{
    // Create a new matrix to return later
    rd_xform return_matrix;

    // Step through the rows of matrix one
    for (int m1_rows = 0; m1_rows < 4; m1_rows++)
    {
        // Step through the columns of matrix two
        for (int m2_cols = 0; m2_cols < 4; m2_cols++)
        {
            // Get the sum of the row * column multiplication
            float sum = 0;

            // Step through the cols of matrix one and the rows of matrix two
            for (int index = 0; index < 4; index++)
                sum += matrix[m1_rows][index] * m2.matrix[index][m2_cols];

            // Store the sum in our return matrix
            return_matrix.matrix[m1_rows][m2_cols] = sum;
        }
    }

    // Return the new matrix
    return return_matrix;
}

/// @param point The homogenous point to multiply into this translation matrix.
rd_pointh rd_xform::operator*(rd_pointh point)
{
    // Create our homogenous point to return
    rd_pointh return_point;

    // Step through each row in our matrix
    for (int rows = 0; rows < 4; rows++)
    {
        // Store the sum of the row x column mult
        float sum = 0;

        // Step through each value and add them to the sum after mult
        for (int index = 0; index < 4; index++)
            sum += matrix[rows][index] * point[index];

        // Set the value in our return point
        return_point[rows] = sum;
    }

    // Return our new point
    return return_point;
}

/// @param m2 The matrix whose data is being copied into this one.
/// @returns A reference to this matrix with its updated values.
rd_xform& rd_xform::operator=(rd_xform m2)
{
    // Copy all the data from m2 into m1
    for (int rows = 0; rows < 4; rows++)
        for (int cols = 0; cols < 4; cols++)
            matrix[rows][cols] = m2.matrix[rows][cols];

    // Return the pointer to this matrix
    return *this;
}

/// Sets the matrix to be a translation matrix, with the given translation
/// values being used for Tx, Ty, and Tz.
/// @param tx The translation to apply to the x-axis.
/// @param ty The translation to apply to the y-axis.
/// @param tz The translation to apply to the z-axis.
void rd_xform::set_translation(float tx, float ty, float tz)
{
    // Reset the matrix
    set_identity();

    // Set the three translation values
    matrix[0][3] = tx; // row 1 col 4
    matrix[1][3] = ty; // row 2 col 4
    matrix[2][3] = tz; // row 3 col 4
}

/// Sets the matrix to be a scale matrix.
/// @param sx The scale to apply to the x-axis.
/// @param sy The scale to apply to the y-axis.
/// @param sz The scale to apply to the z-axis.
void rd_xform::set_scale(float sx, float sy, float sz)
{
    // Reset the matrix
    set_identity();

    // Set the three scalar values
    matrix[0][0] = sx; // row 1 col 1
    matrix[1][1] = sy; // row 2 col 2
    matrix[2][2] = sz; // row 3 col 3
}

/// Sets the matrix to be a rotation matrix about the Z-axis (X towards Y).
/// @param angle The angle in degrees to rotate by.
void rd_xform::set_xy_rotation(float angle)
{
    // Reset the matrix
    set_identity();

    // Convert from degrees to radians
    float radians = angle * (M_PI/180);

    // Set up the matrix for rotation about Z
    matrix[0][0] = cosf(radians); // row 1 col 1
    matrix[0][1] = -sinf(radians); // row 1 col 2
    matrix[1][0] = sinf(radians); // row 2 col 1
    matrix[1][1] = cosf(radians); // row 2 col 2
}

/// Sets the matrix to be a rotation matrix about the X-axis (Y towards Z).
/// @param angle The angle in degrees to rotate by.
void rd_xform::set_yz_rotation(float angle)
{
    // Reset the matrix
    set_identity();

    // Convert from degrees to radians
    float radians = angle * (M_PI/180);

    // Set up the matrix for rotation about X
    matrix[1][1] = cosf(radians);
    matrix[1][2] = -sinf(radians);
    matrix[2][1] = sinf(radians);
    matrix[2][2] = cosf(radians);
}

/// Sets the matrix to be a rotation matrix about the Y-axis (Z towards X).
/// @param angle The angle in degrees to rotate by.
void rd_xform::set_zx_rotation(float angle)
{
    // Reset the matrix
    set_identity();

    // Convert from degrees to radians
    float radians = angle * (M_PI/180);

    // Set up the matrix for rotation about Y
    matrix[0][0] = cosf(radians);
    matrix[0][2] = sinf(radians);
    matrix[2][0] = -sinf(radians);
    matrix[2][2] = cosf(radians);
}

/// Sets the matrix back to the standard identity matrix.
void rd_xform::set_identity()
{
    // Initialize each value in the matrix to 0
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            matrix[row][col] = 0;

    // Set each value on the diagonal to 1
    matrix[0][0] = 1;
    matrix[1][1] = 1;
    matrix[2][2] = 1;
    matrix[3][3] = 1;
}

/// Sets the specified element in the matrix in row x and column y to
/// the desired float.
/// @param y The row from top to bottom to update.
/// @param x The column from left to right to update.
void rd_xform::set_element(int y, int x, float element)
{
    // Make sure our values are in range
    if (x >= 4 || y >= 4)
        throw std::out_of_range("The transformation matrices are only 4x4.");

    // Update the element in our matrix
    matrix[y][x] = element;
}

/// Prints out the matrix to standard output. Used for debugging
/// and checking the contents of a given matrix.
void rd_xform::print_matrix() const
{
    // Print out the matrix by row and column
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
            std::cout << matrix[row][col] << " ";

        std::cout << std::endl;
    }
}
