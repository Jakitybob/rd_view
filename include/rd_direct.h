/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

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

    // 2D Rendering functions (basically deprecated)
    void plot_circle(int x, int y, int xCenter, int yCenter);
    void flood_fill(const float seed_point[3], float seed_color[3]);

    // Graphics Pipeline functions
    void calculate_world_to_clip();
    void calculate_clip_to_device();
    void render_point(class rd_pointh point);
    bool check_point_clip(class rd_pointh point);
    void render_line(class rd_pointh point, bool should_draw);

    // Primitive Rendering functions
    void render_circle(float radius);

public:
    int rd_display(const string &name, const string &type, const string &mode) override;
    int rd_format(int xresolution, int yresolution) override;
    int rd_world_begin() override;
    int rd_world_end() override;
    int rd_frame_begin(int frame_no) override;
    int rd_frame_end() override;
    int rd_camera_eye(const float eyepoint[3]) override;
    int rd_camera_at(const float atpoint[3]) override;
    int rd_camera_up(const float up[3]) override;
    int rd_camera_fov(float fov) override;
    int rd_clipping(float znear, float zfar) override;
    int rd_translate(const float offset[3]) override;
    int rd_scale(const float scale_factor[3]) override;
    int rd_rotate_xy(float angle) override;
    int rd_rotate_yz(float angle) override;
    int rd_rotate_zx(float angle) override;
    int rd_xform_push(void) override;
    int rd_xform_pop(void) override;
    int rd_circle(const float center[3], float radius) override;
    int rd_line(const float start[3], const float end[3]) override;
    int rd_point(const float p[3]) override;
    int rd_cone(float height, float radius, float thetamax) override;
    int rd_cube(void) override;
    int rd_cylinder(float radius, float zmin, float zmax, float thetamax) override;
    int rd_disk(float height, float radius, float theta) override;
    int rd_sphere(float radius, float zmin, float zmax, float thetamax) override;
    int rd_background(const float color[]) override;
    int rd_color(const float color[]) override;
    int rd_fill(const float seed_point[3]) override;
};

#endif /* RD_ENGINE_DIRECT_H */
