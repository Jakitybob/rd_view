/********************************************************************
CSCI 490J - Assignment 1 - Spring 2026
Programmer: Jacob Yates
Date Due: 02/13/2026

Purpose: This file implements some of the rd_direct rendering
        functionality derived from rd_enginebase class. This includes
        drawing lines and circles, as well as flood filling areas to
        a two-dimensional canvas.
*********************************************************************/

#include "rd_direct.h"

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
    for (int x = 0, y = radius; x < y;)
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

/// TODO: Implement
int REDirect::rd_fill(const float seed_point[3])
{
    // If the seed point is out of range, return
    if (seed_point[0] < 0 || seed_point[0] >= display_xSize || seed_point[1] < 0 || seed_point[1] >= display_ySize)
        return RD_OK;

    // Get the color of the current pixel
    float color[3]; // Create a float to store the current pixel color in
    rd_read_pixel(seed_point[0], seed_point[1], color);

    // If the pixel isn't the background color, return. Use absolute value subtraction to allow a margin of error that
    // comes from dealing with floating point numbers from time to time.
    if (std::abs(color[0] - backgroundRed) > 0.05f || std::abs(color[1] - backgroundGreen) > 0.05f || std::abs(color[2] - backgroundBlue) > 0.05f)
        return RD_OK;

    // Otherwise, update the current pixel to the fill color
    rd_write_pixel(seed_point[0], seed_point[1], new float[3] {drawRed, drawGreen, drawBlue});

    // Recursively call fill on the four connected pixels to the current one
    rd_fill(new float[3] {seed_point[0] + 1, seed_point[1], seed_point[2]});
    rd_fill(new float[3] {seed_point[0] - 1, seed_point[1], seed_point[2]});
    rd_fill(new float[3] {seed_point[0], seed_point[1] + 1, seed_point[2]});
    rd_fill(new float[3] {seed_point[0], seed_point[1] - 1, seed_point[2]});

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