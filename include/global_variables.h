/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This file contains static global variables for the program
        to access during runtime and keeps information like the
        current draw color and background color, among other things.
*********************************************************************/

#ifndef CSCI631_PROJECT1_GLOBAL_VARIABLES_H
#define CSCI631_PROJECT1_GLOBAL_VARIABLES_H

#include <stack>
#include "rd_xform.h"
#include "rd_pointc.h"
#include "rd_pointh.h"

// RGB draw color, default white
static float drawRed = 1.0f;
static float drawGreen = 1.0f;
static float drawBlue = 1.0f;

// RGB background color, default black
static float backgroundRed = 0.0f;
static float backgroundGreen = 0.0f;
static float backgroundBlue = 0.0f;

// Transformation matrices
static rd_xform current_transform;
static rd_xform world_to_clip;
static rd_xform clip_to_device;

// Camera information
static rd_pointc camera_eye = rd_pointc(0, 0, 0);
static rd_pointc camera_at = rd_pointc(0, 0, -1);
static rd_pointc camera_up = rd_pointc(0, 1, 0);
static float camera_fov = 90.f;
static float near_clip = 1.0f;
static float far_clip = 1.0e+09;

// Transformation related variables
static std::stack<rd_xform> xform_stack;
static rd_pointh last_vertex; // The last vertex moved to in the line pipeline, in clip coordinates

// Global variable for frame number
static int frame_number;

// Global variable for the array that holds the PNM image data
inline float*** image_array;

#endif //CSCI631_PROJECT1_GLOBAL_VARIABLES_H