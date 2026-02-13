/********************************************************************
CSCI 490J - Assignment 1 - Spring 2026
Programmer: Jacob Yates
Date Due: 02/13/2026

Purpose: This class implements simple rendering algorithms and is
        derived from the RenderEngine class. It draws simple
        primitives such as lines and circles to the screen as
        instructed by the drivers and can also handle flood fill.
*********************************************************************/

#ifndef RD_ENGINE_DIRECT_H
#define RD_ENGINE_DIRECT_H

#include "rd_enginebase.h"
#include "rd_display.h"
#include "rd_error.h"
#include "global_variables.h"
#include <string>
using std::string;
#include <iostream>
#include <ostream>
#include <stack>

// This is a rendering engine that renders directly to the image buffer as
// primitives come in.  A depth buffer is obviously needed.  Transparency is
// not properly handled.

class REDirect: public RenderEngine
{
private:
    int frameNumber = 0; // Current frame
    void plot_steep_line(int startX, int startY, int endX, int endY);
    void plot_shallow_line(int startX, int startY, int endX, int endY);
    void plot_circle(int x, int y, int xCenter, int yCenter);

public:
    int rd_display(const string &name, const string &type, const string &mode) override;
    int rd_format(int xresolution, int yresolution) override;
    int rd_world_begin() override;
    int rd_world_end() override;
    int rd_frame_begin(int frame_no) override;
    int rd_frame_end() override;
    int rd_circle(const float center[3], float radius) override;
    int rd_line(const float start[3], const float end[3]) override;
    int rd_point(const float p[3]) override;
    int rd_background(const float color[]) override;
    int rd_color(const float color[]) override;
    int rd_fill(const float seed_point[3]) override;
};

#endif /* RD_ENGINE_DIRECT_H */
