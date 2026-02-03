#include "rd_direct.h"

#include <iostream>
#include <ostream>

#include "rd_display.h"
#include "rd_error.h"
#include "global_variables.h"

int REDirect::rd_display(const string &name, const string &type, const string &mode)
{


    return RD_OK;
}

int REDirect::rd_format(int xresolution, int yresolution)
{
    return RD_OK;
}

int REDirect::rd_world_begin()
{
    rd_disp_init_frame(frameNumber);
    return RD_OK;
}

int REDirect::rd_world_end()
{
    rd_disp_end_frame();
    return RD_OK;
}

int REDirect::rd_frame_begin(int frame_no)
{
    // Store the frame number globally for use elsewhere
    frameNumber = frame_no;

    return RD_OK;
}

int REDirect::rd_frame_end()
{
    rd_disp_end_frame();
    return RD_OK;
}

int REDirect::rd_circle(const float center[3], float radius)
{
    return RD_OK;
}

int REDirect::rd_line(const float start[3], const float end[3])
{
    // Calculate our slope
    float slope = (end[1] - start[1]) / (end[0] - start[0]);

    //

    return RD_OK;
}

int REDirect::rd_point(const float p[3])
{
    // Draw a single pixel at the desired location with the current draw color
    rd_write_pixel(p[0], p[1], new float[3] {drawRed, drawGreen, drawBlue});

    return RD_OK;
}

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

int REDirect::rd_color(const float color[])
{
    // Store the colors into our global variables
    drawRed = color[0];
    drawGreen = color[1];
    drawBlue = color[2];

    return RD_OK;
}

int REDirect::rd_fill(const float seed_point[3])
{
    return RD_OK;
}
