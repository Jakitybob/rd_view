/********************************************************************
CSCI 490J - Assignment 5 - Spring 2026
Programmer: Jacob Yates
Last Updated: 4/30/2026

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
#include "rd_lights.h"
#include "rd_pointa.h"

#define MAX_LIGHTS 30

// Forward declaration(s)
struct rd_edge;

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
static rd_xform normal_transform;
static rd_xform world_to_clip;
static rd_xform clip_to_device;

// Camera information
static rd_pointc camera_eye = rd_pointc(0, 0, 0); // Where the camera is located physically
static rd_pointc camera_at = rd_pointc(0, 0, -1); // Where the camera is looking towards
static rd_pointc camera_up = rd_pointc(0, 1, 0);
static float camera_fov = 90.f;
static float near_clip = 1.0f;
static float far_clip = 1.0e+09;

// Transformation related variables
static std::stack<rd_xform> xform_stack;
static rd_pointh last_vertex; // The last vertex moved to in the line pipeline, in clip coordinates
static int NUM_SEGMENTS = 20; // The number of segments to use for round objects, default 20

// The depth buffer for handling z-based depth when drawing pixels
static float** depth_buffer = nullptr; // A two-dimensional array
static rd_edge* edge_table = nullptr; // The table for each edge in a scanline

// Lighting-related globals
static ambient_light global_ambient_light;
static far_light far_lights[MAX_LIGHTS];
static int num_far_lights = 0;
static point_light point_lights[MAX_LIGHTS];
static int num_point_lights = 0;
static float ambient_coefficient = 1.f, diffuse_coefficient = 0.f, specular_coefficient = 0.f;
static float surface_color[3] = {1.f, 1.f, 1.f}; // Default to white
static float specular_color[3] = {1.f, 1.f, 1.f}; // Default to white
static int specular_exponent = 10;
static bool vertex_color_flag = false, vertex_normal_flag = false, vertex_texture_flag = false, interpolation_flag = true;
static rd_vector viewing_vector; // Direction from a surface point to the eye of the camera for a given polygon
static rd_vector poly_normal; // Surface normal for a given polygon, may or may not be used
static rd_pointa surface_point_values;
static void (*shader)(float*) = nullptr; // Function pointer for the shaders
static bool use_surface_flag = false; // Whether specular should use surface color or specular color (true for metallic, false for plastic)

// Global variable for frame number
static int frame_number = 0;

// Global variable for the array that holds the PNM image data
inline float*** image_array;

#endif //CSCI631_PROJECT1_GLOBAL_VARIABLES_H