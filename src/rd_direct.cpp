#include "rd_direct.h"

#include <iostream>
#include <ostream>

#include "rd_display.h"
#include "rd_error.h"
#include "global_variables.h"

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

/// TODO: Implement
int REDirect::rd_circle(const float center[3], float radius)
{
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
    // If our slope is less than 1, create a shallow line
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
    // Calculate the deltaX and deltaY
    int deltaX = endX - startX;
    int deltaY = endY - startY;
    int yIncrement = 1; // Used for negative slopes

    // Create our decision variable and store the initial value
    int decision = 2 * deltaY - deltaX;

    // Step across the x-axis to plot points and see if we should update y
    for (int x = startX, y = startY; x < endX; x++)
    {
        rd_write_pixel(x, y, new float[3] {drawRed, drawGreen, drawBlue});
        if (decision >= 0)
        {
            // See if we need to increment or decrement y based on deltaY
            if (deltaY >= 0)
                y++;
            else
                y--;

            // Update our decision variable
            decision += 2 * abs(deltaY) - 2 * deltaX;
        }
        else
        {
            decision += 2 * abs(deltaY);
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

    // Create our decision variable and store the initial value
    int decision = 2 * deltaX - deltaY;

    // Step across the y-axis to plot points and see if we should increment x
    for (int y = startY, x = startX; y < endY; y++)
    {
        rd_write_pixel(x, y, new float[3] {drawRed, drawGreen, drawBlue});
        if (decision >= 0) // If decision >= 0, we should update x.
        {
            // See if x should increment or decrement based on deltaX
            if (deltaX > 0)
                x++;
            else
                x--;

            // Update our decision variable
            decision += 2 * abs(deltaX) - 2 * deltaY;
        }
        else
            decision += 2 * abs(deltaX);
    }
}
