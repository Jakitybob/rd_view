/********************************************************************
CSCI 490J - Assignment 1 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This file implements some of the rd_direct rendering
        functionality derived from rd_enginebase class. This includes
        drawing lines and circles, as well as flood filling areas to
        a two-dimensional canvas.
*********************************************************************/

#include "rd_direct.h"

#include <cmath>
#include "rd_vector.h"
#include "rd_xform.h"

// Simple stub as functionality is handled behind the scenes currently.
int REDirect::rd_display(const string &name, const string &type, const string &mode)
{
    return RD_OK;
}

// Simple stub as functionality is handled behind the scenes currently.
int REDirect::rd_format(int xresolution, int yresolution)
{
    return RD_OK;
}

/// Initializes the display for a new frame.
int REDirect::rd_world_begin()
{
    // Make sure our transform stack is empty, and empty it if not
    while (!xform_stack.empty()) // It should always be empty but better safe than sorry
        xform_stack.pop();

    // Push on a fresh identity matrix to the transform stack
    rd_xform xform;
    xform_stack.push(xform);

    // TODO:
    // * Clipping -> Device

    // Calculate our graphics pipeline matrices
    calculate_world_to_clip();

    // Initialize the frame and return OK
    rd_disp_init_frame(frameNumber);
    return RD_OK;
}

/// Ensures that the last frame is ended properly.
int REDirect::rd_world_end()
{
    rd_disp_end_frame();
    return RD_OK;
}

/// Stores the current frame number out for later.
int REDirect::rd_frame_begin(int frame_no)
{
    // Store the frame number globally for use elsewhere
    frameNumber = frame_no;

    return RD_OK;
}

/// Ensures that the last frame ends properly.
int REDirect::rd_frame_end()
{
    rd_disp_end_frame();
    return RD_OK;
}

/// Store the camera's position into our global variables.
/// @param eyepoint An array of 3 floats that represent the XYZ of the camera position.
int REDirect::rd_camera_eye(const float eyepoint[3])
{
    // Copy over the data to our global array
    camera_eye.set_x(eyepoint[0]);
    camera_eye.set_y(eyepoint[1]);
    camera_eye.set_z(eyepoint[2]);

    return RD_OK;
}

/// Store the camera's look at position into our global variables.
/// @param atpoint An array of 3 floats that represent the XYZ coordinate of where the camera is looking at.
int REDirect::rd_camera_at(const float atpoint[3])
{
    // Copy over the data to our global array
    camera_at.set_x(atpoint[0]);
    camera_at.set_y(atpoint[1]);
    camera_at.set_z(atpoint[2]);

    return RD_OK;
}

/// Store the camera's up vector into our global variables.
/// @param up An array of 3 floats that represent the up vector of the camera.
int REDirect::rd_camera_up(const float up[3])
{
    // Copy the data to our global array
    camera_up.set_x(up[0]);
    camera_up.set_y(up[1]);
    camera_up.set_z(up[2]);

    return RD_OK;
}

/// Store the FOV into our global variables.
/// @param fov The float to set the camera's field of view to.
int REDirect::rd_camera_fov(float fov)
{
    // Store our data into our global variable
    camera_fov = fov;

    return RD_OK;
}

// TODO: IMPLEMENT
int REDirect::rd_clipping(float znear, float zfar)
{
    return RD_OK;
}

/// Implements the midpoint circle algorithm to draw a circle with the desired
/// radius using the provided coordinates as the midpoint for said circle.
/// @param center A const float* to an array of 3 variables representing the X, Y, and Z
///         coordinate of the center of the circle. At present the Z value is ignored.
/// @param radius The radius of the circle, ie the distance from the center to the
///         edge of the circle where the points will be drawn.
int REDirect::rd_circle(const float center[3], float radius)
{
    // Create our decision variable
    int decision = 1 - radius;

    // Loop across our octant to draw our circle
    for (int x = 0, y = radius; x <= y;)
    {
        // Plot our points and increment x
        plot_circle(x, y, center[0], center[1]);
        x++;

        // Check if pixel is outside or on circle
        if (decision <= 0)
            decision += 2 * x + 1;
        // Otherwise pixel is inside circle, decrement y
        else
        {
            y--;
            decision += 2 * x - 2 * y + 1;
        }
    }

    return RD_OK;
}

/// Draws a line on the screen using a derivative of Bresenham's line algorithm. This function
/// calls plot_shallow_line or plot_steep_line based on the slope.
/// @param start A const float* to an array of 3 values representing the x, y, z coordinates
///         of the starting point of the line.
/// @param end A const float* to an array of 3 values representing the x, y, z coordinates
///         of the ending point of the line.
int REDirect::rd_line(const float start[3], const float end[3])
{
    // If our slope is between -1 and 1 plot a shallow line
    if (abs(end[1] - start[1]) < abs(end[0] - start[0]))
    {
        if (start[0] > end[0]) // If the start is greater than the end, flip the start and end points
            plot_shallow_line(end[0], end[1], start[0], start[1]);
        else
            plot_shallow_line(start[0], start[1], end[0], end[1]);
    }
    else // Otherwise create a steep line
    {
        if (start[1] > end[1]) // If the start is greater than the end, flip the start and end points
            plot_steep_line(end[0], end[1], start[0], start[1]);
        else
            plot_steep_line(start[0], start[1], end[0], end[1]);
    }

    return RD_OK;
}

/// Draws a single pixel onto the screen at the desired x, y location.
/// @param p A float* to an array of 3 values representing the x, y, and z coordinate of the point.
///         Note that the z value is currently ignored.
int REDirect::rd_point(const float p[3])
{
    // Draw a single pixel at the desired location with the current draw color
    rd_write_pixel(p[0], p[1], new float[3] {drawRed, drawGreen, drawBlue});

    return RD_OK;
}

/// Updates the global RGB color values for the background color and forces an update to
/// the background color with the new color as well.
/// @param color A float* to an array of (currently) three values representing RGB.
int REDirect::rd_background(const float color[])
{
    // Store the color into our global variables
    backgroundRed = color[0];
    backgroundGreen = color[1];
    backgroundBlue = color[2];

    // Update the background to use our new color
    rd_set_background(new float[3] {backgroundRed, backgroundGreen, backgroundBlue});

    return RD_OK;
}

/// Updates the global RGB color values for drawing to the screen.
/// @param color A float* to an array of (currently) three values representing RGB.
int REDirect::rd_color(const float color[])
{
    // Store the colors into our global variables
    drawRed = color[0];
    drawGreen = color[1];
    drawBlue = color[2];

    return RD_OK;
}

/// Wrapper function for a recursive four-connected flood fill function that will
/// use the color of the initial point provided and flood fill all connected pixels
/// to the draw color.
/// @param seed_point: A const float array with the XYZ coordinates for the point to flood from.
int REDirect::rd_fill(const float seed_point[3])
{
    // If the seed point is out of range, return
    if (seed_point[0] < 0 || seed_point[0] >= display_xSize || seed_point[1] < 0 || seed_point[1] >= display_ySize)
        return RD_OK;

    // Get the color of the first pixel to use as our color to flood over
    float color[3]; // Create a float to store the current pixel color in
    rd_read_pixel(seed_point[0], seed_point[1], color);

    // Start the recursive fill
    flood_fill(seed_point, color);

    return RD_OK;
}

/// Plots a line with a slope between 1 and -1 from the starting point to the
/// ending point using a derivative version of Bresenham's line algorithm.
/// @param startX The X coordinate to use as the starting point for the line.
/// @param startY The Y coordinate to use as the starting point for the line.
/// @param endX The X coordinate to use as the ending point for the line.
/// @param endY The Y coordinate to use as the ending point for the line.
void REDirect::plot_shallow_line(int startX, int startY, int endX, int endY)
{
    // Calculate our deltaX, deltaY, and y increment
    int deltaX = endX - startX;
    int deltaY = endY - startY;
    int yIncrement = 1;

    // If our deltaY is negative, make our yIncrement decrement instead and flip the sign of deltaY.
    if (deltaY < 0)
    {
        yIncrement = -1;
        deltaY = -deltaY;
    }

    // Create our initial decision variable
    int decision = 2 * deltaY - deltaX;

    // Begin the line algorithm stepping through each x and optionally incrementing or decrementing y as we go
    for (int x = startX, y = startY; x <= endX; x++)
    {
        rd_write_pixel(x, y, new float[3] {drawRed, drawGreen, drawBlue}); // Write our current pixel
        if (decision >= 0)
        {
            y += yIncrement;
            decision += 2 * (deltaY - deltaX);
        }
        else
        {
            decision += 2 * deltaY;
        }
    }
}

/// Plots a line with a slope greater than 1 or less than -1 from the starting points
/// to the ending points using a derivative version of Bresenham's line algorithm.
/// @param startX The X value to use as the starting point for the line.
/// @param startY The Y value to use as the starting point for the line.
/// @param endX The X value to use as the ending point for the line.
/// @param endY The Y value to use as the ending point for the line, used as
///         the stop for the included for loop.
void REDirect::plot_steep_line(int startX, int startY, int endX, int endY)
{
    // Calculate deltaX and deltaY
    int deltaX = endX - startX;
    int deltaY = endY - startY;
    int xIncrement = 1;

    // If our deltaX is negative, decrement x instead and negate our deltaX
    if (deltaX < 0)
    {
        xIncrement = -1;
        deltaX = -deltaX;
    }

    // Create our decision variable and store the initial value
    int decision = 2 * deltaX - deltaY;

    // Step across the y-axis to plot points and see if we should increment x
    for (int y = startY, x = startX; y <= endY; y++)
    {
        rd_write_pixel(x, y, new float[3] {drawRed, drawGreen, drawBlue});
        if (decision >= 0) // If decision >= 0, we should update x.
        {
            // Increment / decrement X accordingly
            x += xIncrement;

            // Update our decision variable
            decision += 2 * (deltaX - deltaY);
        }
        else
            decision += 2 * deltaX;
    }
}

/// Plots eight points along the boundary of a circle given an x and y coordinate
/// contained within a single octant of the desired circle.
/// @param x The X value of a point from an octant on the circle.
/// @param y The Y value of a point from an octant on the circle.
/// @param xCenter The x-coordinate of the point at the center of the circle to draw.
/// @param yCenter The y-coordinate of the point at the center of the circle to draw.
void REDirect::plot_circle(int x, int y, int xCenter, int yCenter)
{
    // Array of colors
    const float* color = new float[3] {drawRed, drawGreen, drawBlue};

    // Write all eight points based on different combinations of x, y, -x, and -y.
    rd_write_pixel(x + xCenter, y + yCenter, color);
    rd_write_pixel(-x + xCenter, y + yCenter, color);
    rd_write_pixel(x + xCenter, -y + yCenter, color);
    rd_write_pixel(-x + xCenter, -y + yCenter, color);
    rd_write_pixel(y + xCenter, x + yCenter, color);
    rd_write_pixel(-y + xCenter, x + yCenter, color);
    rd_write_pixel(y + xCenter, -x + yCenter, color);
    rd_write_pixel(-y + xCenter, -x + yCenter, color);
}

/// A recursive function that floods all four-connected pixels that match the seed color
/// that goes until there is nothing left to fill anymore. This can be because it hit a pixel
/// that doesn't match the seed color, or it ran out of the bounds of the rendered space.
/// @param seed_point: A const float array with the XYZ coordinates for the point to flood from.
/// @param seed_color: A const float array with the RGB values of the color to replace.
void REDirect::flood_fill(const float seed_point[3], float seed_color[3])
{
    // If the seed point is out of range, return
    if (seed_point[0] < 0 || seed_point[0] >= display_xSize || seed_point[1] < 0 || seed_point[1] >= display_ySize)
        return;

    // Get the color of the current pixel
    float color[3]; // Create a float to store the current pixel color in
    rd_read_pixel(seed_point[0], seed_point[1], color);

    // If the pixel isn't the seed color, return. Use absolute value subtraction to allow a margin of error that
    // comes from dealing with floating point numbers from time to time.
    if (std::abs(seed_color[0] - color[0]) > 0.01f || std::abs(seed_color[1] - color[1]) > 0.01f || std::abs(seed_color[2] - color[2]) > 0.01f)
        return;

    // Otherwise, update the current pixel to the fill color
    rd_write_pixel(seed_point[0], seed_point[1], new float[3] {drawRed, drawGreen, drawBlue});

    // Recursively call fill on the four connected pixels to the current one
    flood_fill(new float[3] {seed_point[0] + 1, seed_point[1], seed_point[2]}, seed_color);
    flood_fill(new float[3] {seed_point[0] - 1, seed_point[1], seed_point[2]}, seed_color);
    flood_fill(new float[3] {seed_point[0], seed_point[1] + 1, seed_point[2]}, seed_color);
    flood_fill(new float[3] {seed_point[0], seed_point[1] - 1, seed_point[2]}, seed_color);
}

/// A function that calculates the world to clip transformation matrix and
/// stores it our into our global world to clip matrix variable.
void REDirect::calculate_world_to_clip()
{
    // Create our camera vectors using its position, look-at, and up vector.
    rd_vector eye = rd_vector(camera_eye.get_x(), camera_eye.get_y(), camera_eye.get_z());
    rd_vector forward = (camera_at - camera_eye).normalized();
    rd_vector right = (rd_vector(0, 1, 0) * forward).normalized(); // Use world up to calculate right
    rd_vector up = forward * right;

    // Create the view matrix using these vectors
    rd_xform view_matrix = {
        right.GetX(), right.GetY(), right.GetZ(), -(right ^ eye),
        up.GetX(), up.GetY(), up.GetZ(), -(up ^ eye),
        forward.GetX(), forward.GetY(), forward.GetZ(), -(forward ^ eye),
        0, 0, 0, 1
    };

    // Create our perspective transform matrix
    float fov_scale = 1/std::tanf((camera_fov/2) * (float)(std::numbers::pi/180)); // Our FOV scalar
    rd_xform perspective_matrix = {
        fov_scale, 0, 0, 0,
        0, fov_scale, 0, 0,
        0, 0, -(far_clip/(far_clip - near_clip)), -1,
        0, 0, -((far_clip * near_clip)/(far_clip - near_clip)), 0
    };

    // Store our final world to clip matrix by finding the cross product perspective x view
    world_to_clip = perspective_matrix * view_matrix;
}

/// A function that calculates the clip to device transformation matrix
/// and stores it directly into our global matrix variable
void REDirect::calculate_clip_to_device()
{
    // Create our clip to device matrix and store it into our gobal variable
    clip_to_device = {
        (float)(display_xSize / 2), 0, 0, (float)(display_xSize / 2),
        0, (float)(display_ySize / 2), 0, (float)(display_ySize / 2),
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}
