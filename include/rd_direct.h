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

#include "rd_pointa.h"
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
    // 2D Rendering functions (basically deprecated)
    void plot_circle(int x, int y, int xCenter, int yCenter);
    void flood_fill(const float seed_point[3], float seed_color[3]);

    // Graphics Pipeline functions
    void plot_pixel(class rd_pointc point, float* color);
    void calculate_world_to_clip();
    void calculate_clip_to_device();
    void render_point(class rd_pointh point);
    bool check_point_clip(class rd_pointh point);
    void render_line(class rd_pointh point, bool should_draw);
    void clip_line(class rd_pointh point, bool should_draw);
    void plot_line(class rd_pointh point);
    int render_poly(rd_pointa point, bool should_draw);
    void draw_poly(int num_vertex, rd_pointa* clipped_list);

    // Functions for scan conversion inside of draw_poly
    bool build_edge_list(int num_vertex, rd_pointa* points);
    void make_edge_record(rd_pointa v1, rd_pointa v2);
    void add_active_list(int scanline, rd_edge* aet);
    void insert_edge(rd_edge* list, rd_edge* edge);
    void update_aet(int scanline, rd_edge* aet);
    void delete_next_edge(rd_edge* edge);
    void resort_aet(rd_edge* aet);
    void fill_between_edges(int scanline, rd_edge* aet);

    // Functions for polygon clipping TODO: move stuff out of here into their own classes, this is TOO BIG
    int clip_poly(int num_vertex, rd_pointa* vertex_list, rd_pointa* clipped_list);
    void clip_vertex(class rd_pointa vertex, int boundary, rd_pointa* first_array, rd_pointa* last_array, bool* flag_array, rd_pointa* clipped_list, int* vertex_count);
    void clip_last_vertex(rd_pointa* first_array, rd_pointa* last_array, bool* flag_array, rd_pointa* clipped_list, int* vertex_count);
    bool inside_boundary(class rd_pointa, int boundary);
    bool crosses_boundary(class rd_pointa v1, class rd_pointa v2, int boundary);
    rd_pointa boundary_intersection(rd_pointa v1, rd_pointa v2, int boundary);

    // Primitive Rendering functions
    void render_circle(float radius, float z);

    // Utility functions
    float clamp(float val, float min, float max);

public:
    // GENERAL FUNCTIONS
    int rd_display(const string &name, const string &type, const string &mode) override;
    int rd_format(int xresolution, int yresolution) override;
    int rd_world_begin() override;
    int rd_world_end() override;
    int rd_frame_begin(int frame_no) override;
    int rd_frame_end() override;
    int rd_render_cleanup(void) override;

    // CAMERA
    int rd_camera_eye(const float eyepoint[3]) override;
    int rd_camera_at(const float atpoint[3]) override;
    int rd_camera_up(const float up[3]) override;
    int rd_camera_fov(float fov) override;
    int rd_clipping(float znear, float zfar) override;

    // TRANSFORMATIONS
    int rd_translate(const float offset[3]) override;
    int rd_scale(const float scale_factor[3]) override;
    int rd_rotate_xy(float angle) override;
    int rd_rotate_yz(float angle) override;
    int rd_rotate_zx(float angle) override;
    int rd_xform_push(void) override;
    int rd_xform_pop(void) override;

    // GEOMETRIC OBJECTS
    int rd_circle(const float center[3], float radius) override;
    int rd_line(const float start[3], const float end[3]) override;
    int rd_point(const float p[3]) override;
    int rd_pointset(const string & vertex_type, int nvertex, const vector<float> & vertex) override;
    int rd_polyset(const string & vertex_type, int nvertex, const vector<float> & vertex,
                            int nface,   const vector<int> & face) override;
    int rd_cone(float height, float radius, float thetamax) override;
    int rd_cube(void) override;
    int rd_cylinder(float radius, float zmin, float zmax, float thetamax) override;
    int rd_disk(float height, float radius, float theta) override;
    int rd_sphere(float radius, float zmin, float zmax, float thetamax) override;

    // LIGHTING & SHADING
    int rd_background(const float color[]) override;
    int rd_color(const float color[]) override;
    int rd_fill(const float seed_point[3]) override;
    int rd_surface(const string & shader_type) override;
    int rd_point_light(const float pos[3], const float color[], float intensity) override;
    int rd_far_light  (const float dir[3], const float color[], float intensity) override;
    int rd_ambient_light(const float color[], float intensity) override;
    int rd_k_ambient(float Ka) override;
    int rd_k_diffuse(float Kd) override;
    int rd_k_specular(float Ks) override;
    static void matte_shader(float* color);
    static void metal_shader(float* color);
    static void plastic_shader(float* color);

    // OPTIONS
    int rd_option_bool(const string& name, bool flag);
};

#endif /* RD_ENGINE_DIRECT_H */
