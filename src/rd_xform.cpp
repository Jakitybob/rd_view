//
// Created by jake on 3/10/26.
//

#include "rd_xform.h"
#include <cmath>
#include <iostream>

/// @returns A matrix initialized to the default identity matrix.
rd_xform::rd_xform()
{
    // Set the matrix to an identity matrix
    set_identity();
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

    // Set up the matrix for rotation about Z
    matrix[0][0] = cosf(angle); // row 1 col 1
    matrix[0][1] = -sinf(angle); // row 1 col 2
    matrix[1][0] = sinf(angle); // row 2 col 1
    matrix[1][1] = cosf(angle); // row 2 col 2
}

void rd_xform::set_yz_rotation(float angle)
{

}

void rd_xform::set_zx_rotation(float angle)
{

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

/// Prints out the matrix to standard output. Used for debugging
/// and checking the contents of a given matrix.
void rd_xform::print_matrix()
{
    // Print out the matrix by row and column
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
            std::cout << matrix[row][col] << " ";

        std::cout << std::endl;
    }
}
