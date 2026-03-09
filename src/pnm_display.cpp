/********************************************************************
CSCI 490J - Assignment 1 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This file handles implementing the pnm_display functions
        that allow the output of an image contained in the global
        image_array variable out to a .ppm file. This class does
        not handle any drawing algorithms, only the handling of
        outputting each frame to an image.
*********************************************************************/

#include "pnm_display.h"
#include "rd_error.h"
#include "global_variables.h"
#include "rd_display.h"
#include <fstream>
using std::ofstream;
#include <iostream>
using std::ios;

/// Allocates the memory for the image_array global variable.
int pnm_init_display()
{
    // Allocate memory for our image display array
    image_array = new float**[display_xSize];
    for (int index = 0; index < display_xSize; index++) // Iterate through each row to create each column
    {
        image_array[index] = new float*[display_ySize];
        for (int sub_index = 0; sub_index < display_ySize; sub_index++) // Iterate through each point to allocate space for the RGB values
        {
            image_array[index][sub_index] = new float[3];
        }
    }

    return RD_OK;
}

/// Frees up allocated memory from the image_array global variable.
int pnm_end_display()
{
    // Free each allocated column in our image array
    for (int index = 0; index < display_xSize; index++)
    {
        // Free each point of memory in our array first
        for (int sub_index = 0; sub_index < display_ySize; sub_index++)
            delete image_array[index][sub_index];

        // Free the rest of the column
        delete image_array[index];
    }

    // Free up the memory allocated for our rows in our image array
    delete image_array;

    return RD_OK;
}

/// Stores the frame number and clears the display to the background color for a new frame.
/// @param _frame_number The integer that represents the current frame number.
int pnm_init_frame(int _frame_number)
{
    // Store the frame number
    frame_number = _frame_number;

    // Clear out the image array back to the bg color
    pnm_clear();

    return RD_OK;
}

/// Outputs the current frame to a .ppm file using the display name from the .rd instructions
/// as well as an appended number based on what the current frame number is.
int pnm_end_frame()
{
    // Create a fstream object and open a file with the display name
    ofstream out_file;
    out_file.open(display_name + std::to_string(frame_number) + ".ppm", ios::binary);

    // Print out the header for the PPM file
    out_file << "P6\n" << display_xSize << " " << display_ySize << "\n";
    out_file << "255\n";

    // Iterate through each row in our image array (Y)
    for (int index = 0; index < display_ySize; index++)
    {
        // Iterate through each column in our image array (X)
        for (int sub_index = 0; sub_index < display_xSize; sub_index++)
        {
            // Put our stored data in our image array to the frame
            out_file.put(image_array[sub_index][index][0] * 255); // RED
            out_file.put(image_array[sub_index][index][1] * 255); // GREEN
            out_file.put(image_array[sub_index][index][2] * 255); // BLUE
        }
    }

    // Close the out file
    out_file.close();

    return RD_OK;
}

/// Sets the pixel at the desired coordinate in the image_array to the specified RGB value.
/// @param x The x-coordinate of the pixel to update.
/// @param y The y-coordinate of the pixel to update.
/// @param rgb A constant float* that contains 3 values representing R, G, and B.
int pnm_write_pixel(int x, int y, const float rgb[])
{
    // Set our color at the desired point
    image_array[x][y][0] = rgb[0];
    image_array[x][y][1] = rgb[1];
    image_array[x][y][2] = rgb[2];

    return RD_OK;
}

/// Sets the RGB values in the provided array to the color currently stored at the pixel at (x, y).
/// @param x The x-coordinate of the pixel to read.
/// @param y The y-coordinate of the pixel to read.
/// @param rgb A float array of size 3 where the color information will be stored.
int pnm_read_pixel(int x, int y, float rgb[])
{
    // Store our current RBG values from our image array into the out array
    rgb[0] = image_array[x][y][0];
    rgb[1] = image_array[x][y][1];
    rgb[2] = image_array[x][y][2];

    return RD_OK;
}

/// Sets the global background to the provided color.
/// @param rgb A float array of size 3 that contains RGB color values between 0 and 1.
int pnm_set_background(const float rgb[])
{
    backgroundRed = rgb[0];
    backgroundGreen = rgb[1];
    backgroundBlue = rgb[2];

    return RD_OK;
}

/// Clears the display by setting each individual pixel back to the stored background color.
int pnm_clear()
{
    // Iterate through each row (Y)
    for (int index = 0; index < display_ySize; index++)
    {
        // Iterate through each column (X)
        for (int sub_index = 0; sub_index < display_xSize; sub_index++)
        {
            // Update each pixel to the background color in our image array
            image_array[sub_index][index][0] = backgroundRed;
            image_array[sub_index][index][1] = backgroundGreen;
            image_array[sub_index][index][2] = backgroundBlue;
        }

    }

    return RD_OK;
}
