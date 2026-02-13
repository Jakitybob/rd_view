/********************************************************************
CSCI 490J - Assignment 1 - Spring 2026
Programmer: Jacob Yates
Date Due: 02/13/2026

Purpose: This file contains static global variables for the program
        to access during runtime and keeps information like the
        current draw color and background color, among other things.
*********************************************************************/

#ifndef CSCI631_PROJECT1_GLOBAL_VARIABLES_H
#define CSCI631_PROJECT1_GLOBAL_VARIABLES_H

// Global variables for the RGB draw color, default white
static float drawRed = 1.0f;
static float drawGreen = 1.0f;
static float drawBlue = 1.0f;

// Global variables for the RGB background color, default black
static float backgroundRed = 0.0f;
static float backgroundGreen = 0.0f;
static float backgroundBlue = 0.0f;

// Global variable for frame number
static int frame_number;

// Global variable for the array that holds the PNM image data
inline float*** image_array;

#endif //CSCI631_PROJECT1_GLOBAL_VARIABLES_H