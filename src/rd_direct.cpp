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

#include <algorithm>
#include <bitset>
#include <cmath>

#include "rd_edge.h"
#include "rd_vector.h"
#include "rd_xform.h"
#include "rd_pointc.h"
#include "rd_pointh.h"
#include "rd_pointa.h"
#include "rd_lights.h"

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

/// Initializes the display for a new frame. This includes calculating
/// our world->clip and clip->device transform matrices as well as
/// initializing our depth buffer.
int REDirect::rd_world_begin()
{
    // Reset current transform and normal transform to identity matrices.
    current_transform = rd_xform();
    normal_transform = rd_xform();

    // Make sure our transform stack is empty, and empty it if not
    while (!xform_stack.empty()) // It should always be empty but better safe than sorry
        xform_stack.pop();

    // Push on a fresh identity matrix to the transform stack
    rd_xform xform;
    xform_stack.push(xform);

    // Calculate our graphics pipeline matrices
    calculate_world_to_clip();
    calculate_clip_to_device();

    // Initialize the depth buffer to the screen size
    if (depth_buffer == nullptr)
    {
        depth_buffer = new float*[display_ySize];
        for (int index = 0; index < display_ySize; index++)
            depth_buffer[index] = new float[display_xSize];
    }

    // Clear out the depth buffer by initializing all values to one
    for (int index = 0; index < display_ySize; index++)
        for (int sub_index = 0; sub_index < display_xSize; sub_index++)
            depth_buffer[index][sub_index] = 1;

    // Initialize the edge table
    if (edge_table == nullptr)
        edge_table = new rd_edge[display_ySize];

    // Default to using the matte shader for all rendering
    shader = matte_shader;

    // Create a default white ambient light
    rd_ambient_light(new float[3] {1.0, 1.0, 1.0}, 1.0);

    // Initialize the frame and return OK
    rd_disp_init_frame(frame_number);
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
    frame_number = frame_no;

    // Reset lighting information
    rd_ambient_light(new float[3] {1.0, 1.0, 1.0}, 1.0);
    num_far_lights = 0;
    num_point_lights = 0;

    return RD_OK;
}

/// Ensures that the last frame ends properly.
int REDirect::rd_frame_end()
{
    rd_disp_end_frame();
    return RD_OK;
}

/// Cleans up allocated memory at the end of program runtime.
int REDirect::rd_render_cleanup()
{
    // Free each column of the depth buffer's memory
    for (int index = 0; index < display_ySize; index++)
        delete[] depth_buffer[index];

    // Free the memory for the rows of the depth buffer
    delete[] depth_buffer;

    // Free up the memory for the edge table
    delete[] edge_table;

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

/// Stores the clipping planes into global variables.
/// @param znear The value of the near clipping plane.
/// @param zfar The value of the far clipping plane.
int REDirect::rd_clipping(float znear, float zfar)
{
    // Store our near and far clipping planes globally
    near_clip = znear;
    far_clip = zfar;

    return RD_OK;
}

/// Adds an XYZ translation to the current transformation in the
/// global variables.
/// @param offset An array of 3 floats to translate object space by.
int REDirect::rd_translate(const float offset[3])
{
    // Create the translation matrix with our provided translation values
    rd_xform translation;
    translation.set_translation(offset[0], offset[1], offset[2]);

    // Multiply this translation matrix by our current transform and store it back into the global transform
    current_transform = current_transform * translation;

    return RD_OK;
}

/// Adds a scalar transformation to the current transformation in the
/// global variables.
/// @param scale_factor An array of 3 floats that contain the XYZ scalars.
int REDirect::rd_scale(const float scale_factor[3])
{
    // Create our scale matrix with our provided scalar values
    rd_xform scale;
    scale.set_scale(scale_factor[0], scale_factor[1], scale_factor[2]);

    // Multiply this scaling matrix by our current transform and store it back into the global transform
    current_transform = current_transform * scale;

    // Inverse the scale (1/s) and multiply it by our normal transform
    scale.set_scale(1/scale_factor[0], 1/scale_factor[1], 1/scale_factor[2]);
    normal_transform = scale * normal_transform;

    return RD_OK;
}

/// Adds a rotation about the Z-axis to the current transformation
/// in the global variable transform.
/// @param angle The angle in degrees to rotate by.
int REDirect::rd_rotate_xy(float angle)
{
    // Create our XY rotation matrix with our angle
    rd_xform rotation;
    rotation.set_xy_rotation(angle);

    // Add the rotation to our current transform
    current_transform = current_transform * rotation;

    // Add the inverse of our rotation to the normal transform
    rotation.set_xy_inverse(angle);
    normal_transform = rotation * normal_transform;

    return RD_OK;
}

/// Adds a rotation about the X-axis to the current transformation
/// in the global variable transform.
/// @param angle The angle in degrees to rotate by.
int REDirect::rd_rotate_yz(float angle)
{
    // Create our YZ rotation matrix with our angle
    rd_xform rotation;
    rotation.set_yz_rotation(angle);

    // Add the rotation to our current transform
    current_transform = current_transform * rotation;

    // Add the inverse of our rotation to the normal transform
    rotation.set_yz_inverse(angle);
    normal_transform = rotation * normal_transform;

    return RD_OK;
}

/// Adds a rotation about the Y-axis to the current transformation
/// in the global variable transform.
/// @param angle The angle in degrees to rotate by.
int REDirect::rd_rotate_zx(float angle)
{
    // Create our ZX rotation matrix with our angle
    rd_xform rotation;
    rotation.set_zx_rotation(angle);

    // Add the rotation to our current transform
    current_transform = current_transform * rotation;

    // Add the inverse of our rotation to the normal transform
    rotation.set_zx_inverse(angle);
    normal_transform = rotation * normal_transform;

    return RD_OK;
}

/// Pushes the current transform and normal transform
/// onto the stack, one after the other.
int REDirect::rd_xform_push()
{
    // Push both matrices onto the stack together
    xform_stack.push(current_transform);
    xform_stack.push(normal_transform);

    return RD_OK;
}

/// Sets the normal transform to the top of the
/// stack and the current transform to what is
/// directly below that, since they are always
/// pushed together.
int REDirect::rd_xform_pop()
{
    // Update the normal transform and pop it off the top
    normal_transform = xform_stack.top();
    xform_stack.pop();

    // Do the same for the current transform
    current_transform = xform_stack.top();
    xform_stack.pop();

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

/// Renders a line in 3D space from the starting point to the ending point. The points
/// are passed one by one into the line pipeline, and the second point draws the line between
/// the two.
/// @param start A const float* to an array of 3 values representing the x, y, z coordinates
///         of the starting point of the line.
/// @param end A const float* to an array of 3 values representing the x, y, z coordinates
///         of the ending point of the line.
int REDirect::rd_line(const float start[3], const float end[3])
{
    // Covert each point to a homogenous point
    rd_pointh start_point(start[0], start[1], start[2]);
    rd_pointh end_point(end[0], end[1], end[2]);

    // Pass each point individually through the line pipeline, drawing at the endpoint
    render_line(start_point, false);
    render_line(end_point, true);

    return RD_OK;
}

/// Draws a single pixel onto the screen, running the point through
/// the rendering pipeline and drawing it accordingly.
/// @param p The point in world space to draw the point.
int REDirect::rd_point(const float p[3])
{
    // Convert our point into a homogenous point
    rd_pointh point(p[0], p[1], p[2]);

    // Pass our point through the point pipeline to render it
    render_point(point);

    return RD_OK;
}

/// Creates points from the provided array and passes them into the point pipeline
/// for rendering to the screen.
/// @param vertex_type UNUSED.
/// @param nvertex The number of vertices in the array.
/// @param vertex A reference to an array of floats with the XYZ values to draw.
int REDirect::rd_pointset(const string &vertex_type, int nvertex, const vector<float> &vertex)
{
    // Loop through each vertex, extracting the 3 floats from the array in the process
    for (int index = 0; index < nvertex; index++)
    {
        // Create our point from the next three values in the vector
        rd_pointh point(vertex[0 + index * 3], vertex[1 + index * 3],  vertex[2 + index * 3]);

        // Pass the point into the point pipeline for rendering
        render_point(point);
    }

    return RD_OK;
}

/// Creates a set of polygons based on the information passed in,
/// looping through each face and passing each point to the polygon pipeline.
/// @param vertex_type UNUSED.
/// @param nvertex The number of vertices in the array.
/// @param vertex A reference to an array of floats that contain all vertex information.
/// @param nface The number of faces
/// @param face
int REDirect::rd_polyset(const string &vertex_type, int nvertex, const vector<float> &vertex, int nface, const vector<int> &face)
{
    // Turn off vertex normals as we don't make them here
    vertex_normal_flag = false;

    // Track three consecutive vertices for poly_normal calculation
    int num_vertices = 0;
    rd_pointc consecutive_vertex[3];

    // Loop through each face to draw each line in the connected set.
    for (int index = 0; index < face.size(); index++)
    {
        // Create the attributed point for this vertex
        rd_pointa point(vertex[0 + face[index] * 3], vertex[1 + face[index] * 3], vertex[2 + face[index] * 3], 1);

        // If we don't have three consecutive vertices, add this as one of them
        if (num_vertices < 3)
        {
            consecutive_vertex[num_vertices] = rd_pointc(point);
            num_vertices++;
        }

        // If the vertex number is -1, pass the last vertex in and draw
        if (face[index + 1] == -1)
        {
            // Use the three vertices we saved to calculate the poly normal
            poly_normal = ((consecutive_vertex[1] - consecutive_vertex[0]) * (consecutive_vertex[2] - consecutive_vertex[0])).normalized();

            render_poly(point, true);
            index++; // Double increment
            num_vertices = 0; // Reset our vertex count
            continue;
        }

        // Otherwise pass in our vertex and move on
        render_poly(point, false);
    }

    return RD_OK;
}

/// Draws a cone with the base centered on the XY plane and the
/// tip being height away from the origin at (0, 0, height).
/// @param height The height of the cone.
/// @param radius The radius of the base of the cone.
/// @param thetamax UNUSED.
int REDirect::rd_cone(float height, float radius, float thetamax)
{
    // Enable our vertex normals
    vertex_normal_flag = true;

    // Create our angle to use while creating segments
    float angle = 0; // In radians

    // Draw each segment's face along the edge of the circle at the base
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        // Create the first vertex at our current angle
        rd_pointa v1 = {radius * cosf(angle), radius * sinf(angle), 0, 1};
        v1.set_normal(v1.coord[0], v1.coord[1], radius / height);

        // Update the angle to the next step and add the other side of the base
        angle = index * 2 * M_PI/NUM_SEGMENTS;
        rd_pointa v2 = {radius * cosf(angle), radius * sinf(angle), 0, 1};
        v2.set_normal(v2.coord[0], v2.coord[1], radius / height);

        // Create the vertex at the top and calculate the poly normal
        rd_pointa v3 = {0, 0, height, 1};
        v3.set_normal(0, 0, 1);
        poly_normal = ((rd_pointc(v2) - rd_pointc(v1)) * (rd_pointc(v3) - rd_pointc(v1))).normalized();

        // Pass the three points into the poly pipeline, passing in v3 twice for four total vertices
        render_poly(v1, false);
        render_poly(v2, false);
        render_poly(v3, false);
        render_poly(v3, true);
    }

    return RD_OK;
}

/// Draws a cube centered about the origin with a length and height of one on each
/// side. Each line of the cube is passed into the line pipeline where it will be
/// transformed and sent through the world -> device pipeline.
int REDirect::rd_cube()
{
    // Turn off our vertex normals
    vertex_normal_flag = false;

    // Create the points for the bottom face of the cube
    rd_pointa v1 = {-1, -1, 1, 1};
    rd_pointa v2 = {-1, -1, -1, 1};
    rd_pointa v3 = {1, -1, -1, 1};
    rd_pointa v4 = {1, -1, 1, 1};
    poly_normal = {0, -1, 0};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    // Create the points for the back face of the cube
    v1 = {1, -1, -1, 1};
    v2 = {-1, -1, -1, 1};
    v3 = {-1, 1, -1, 1};
    v4 = {1, 1, -1, 1};
    poly_normal = {0, 0, -1};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    // Create the points for the right face of the cube
    v1 = {1, -1, -1, 1};
    v2 = {1, 1, -1, 1};
    v3 = {1, 1, 1, 1};
    v4 = {1, -1, 1, 1};
    poly_normal = {1, 0, 0};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    // Create the points for the front face of the cube
    v1 = {-1, -1, 1, 1};
    v2 = {-1, 1, 1, 1};
    v3 = {1, 1, 1, 1};
    v4 = {1, -1, 1, 1};
    poly_normal = {0, 0, 1};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    // Create the points for the left face of the cube
    v1 = {-1, -1, 1, 1};
    v2 = {-1, -1, -1, 1};
    v3 = {-1, 1, -1, 1};
    v4 = {-1, 1, 1, 1};
    poly_normal = {-1, 0, 0};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    // Create the points for the top face of the cube
    v1 = {-1, 1, -1, 1};
    v2 = { 1, 1, -1, 1};
    v3 = { 1, 1,  1, 1};
    v4 = {-1, 1,  1, 1};
    poly_normal = {0, 1, 0};

    // Pass the face into the polygon pipeline
    draw_face(v1, v2, v3, v4);

    return RD_OK;
}

/// Draws a cylinder of the given radius with height zmax - zmin.
/// @param radius The radius of the cylinder.
/// @param zmin The z-coordinate of the bottom of the cylinder.
/// @param zmax The z-coordinate of the top of the cylinder.
/// @param thetamax UNUSED.
int REDirect::rd_cylinder(float radius, float zmin, float zmax, float thetamax)
{
    // Turn on the vertex normal flag
    vertex_normal_flag = true;

    // Create our angle to use while creating segments
    float angle = 0; // In radians

    // Draw each segment's face along the edge of the circles
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        float last_angle = angle;
        angle = index * 2 * M_PI/NUM_SEGMENTS; // Move angle to the next step

        // Create each point for the current face of the polygon
        rd_pointa v1 = {radius * cosf(last_angle), radius * sinf(last_angle), zmin, 1};
        v1.set_normal(v1.coord[0] / radius, v1.coord[1] / radius, 0);
        rd_pointa v2 = {radius * cosf(angle), radius * sinf(angle), zmin, 1};
        v2.set_normal(v2.coord[0] / radius, v2.coord[1] / radius, 0);
        rd_pointa v3 = {radius * cosf(angle), radius * sinf(angle), zmax, 1};
        v3.set_normal(v3.coord[0] / radius, v3.coord[1] / radius, 0);
        rd_pointa v4 = {radius * cosf(last_angle), radius * sinf(last_angle), zmax, 1};
        v4.set_normal(v4.coord[0] / radius, v4.coord[1] / radius, 0);

        // Calculate the poly normal using three points
        poly_normal = ((rd_pointc(v2) - rd_pointc(v1)) * (rd_pointc(v3) - rd_pointc(v1))).normalized();

        // Pass each point into the poly pipeline for rendering
        render_poly(v1, false);
        render_poly(v2, false);
        render_poly(v3, false);
        render_poly(v4, true);
    }

    return RD_OK;
}

/// Renders a flat disk parallel to the XY axis and centered on the Z-axis, with the provided radius
/// and at the height in Z as specified.
/// @param height The z value to render the disk at.
/// @param The radius of the disk.
/// @param theta UNUSED.
int REDirect::rd_disk(float height, float radius, float theta)
{
    // Create a float for our angle
    float angle = 0; // In radians

    // Draw each slice around the disk
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        float last_angle = angle;
        angle = index * 2 * M_PI/NUM_SEGMENTS;

        // Create the three (but last is used twice) points for the disk
        rd_pointa v1 = {radius * cosf(angle), radius * sinf(angle), height, 1};
        v1.set_normal(0, 0, 1);
        rd_pointa v2 = {radius * cosf(last_angle), radius * sinf(last_angle), height, 1};
        v2.set_normal(0, 0, 1);
        rd_pointa v3 = {0, 0, height, 1};
        v3.set_normal(0, 0, 1);

        // Calculate the poly normal
        poly_normal = ((rd_pointc(v2) - rd_pointc(v1)) * (rd_pointc(v3) - rd_pointc(v1))).normalized();

        // Pass the three vertices into the poly pipeline, rendering v3 twice for a "cone" shape of the poly
        render_poly(v1, false);
        render_poly(v2, false);
        render_poly(v3, false);
        render_poly(v3, true);
    }

    return RD_OK;
}

/// Draws a sphere using a latitudinal / longitudinal representation
/// of the sphere where it gets broken up into polygons all along
/// the sphere.
/// @radius The radius of the sphere to draw.
/// @param zmin UNUSED.
/// @param zmax UNUSED.
/// @param thetamax UNUSED.
int REDirect::rd_sphere(float radius, float zmin, float zmax, float thetamax)
{
    // Disable the vertex color and texture flags, and enable the normal flag
    vertex_color_flag = false;
    vertex_texture_flag = false;
    vertex_normal_flag = true;

    // Number of latitudinal and longitudinal slices to draw
    int numLatitude = 10;
    int numLongitude = 20;

    // Loop through the latitudinal slices
    for (int latitude = 0; latitude < numLatitude; latitude++)
    {
        // Create two theta variables
        float theta = -M_PI / 2 + latitude * (M_PI / numLatitude);
        float thetaNext = -M_PI / 2 + (latitude + 1) * (M_PI / numLatitude); // The theta for the next step

        // Loop through our longitudinal slices
        for (int longitude = 0; longitude < numLongitude; longitude++)
        {
            // Create the two phi variables
            float phi = longitude * (2 * M_PI / numLongitude);
            float phiNext = (longitude + 1) * (2 * M_PI / numLongitude);

            // Create and pass in each point while calculating their normals
            rd_pointa bottom_left = rd_pointa(radius * cosf(theta) * cosf(phi), radius * cosf(theta) * sinf(phi), radius * sinf(theta), 1);
            bottom_left.set_normal(bottom_left.coord[0], bottom_left.coord[1], bottom_left.coord[2]); // Normals for a circle vertex are the same as the position vector
            rd_pointa top_left = rd_pointa(radius * cosf(thetaNext) * cosf(phi), radius * cosf(thetaNext) * sinf(phi), radius * sinf(thetaNext), 1);
            top_left.set_normal(top_left.coord[0], top_left.coord[1], top_left.coord[2]); // Normals for a circle vertex are the same as the position vector
            rd_pointa top_right = rd_pointa(radius * cosf(thetaNext) * cosf(phiNext), radius * cosf(thetaNext) * sinf(phiNext), radius * sinf(thetaNext), 1);
            top_right.set_normal(top_right.coord[0], top_right.coord[1], top_right.coord[2]); // Normals for a circle vertex are the same as the position vector
            rd_pointa bottom_right = rd_pointa(radius * cosf(theta) * cosf(phiNext), radius * cosf(theta) * sinf(phiNext), radius * sinf(theta), 1);
            bottom_right.set_normal(bottom_right.coord[0], bottom_right.coord[1], bottom_right.coord[2]); // Normals for a circle vertex are the same as the position vector

            // Pass each vertex into the poly pipeline, drawing counter-clockwise looking from the outside
            render_poly(bottom_left, false);
            render_poly(top_left, false);
            render_poly(top_right, false);

            // Hold three cartesian coordinates for our vertices
            rd_pointc v1 = rd_pointc(bottom_left.coord[0], bottom_left.coord[1], bottom_left.coord[2]);
            rd_pointc v2 = rd_pointc(top_left.coord[0], top_left.coord[1], top_left.coord[2]);
            rd_pointc v3 = rd_pointc(top_right.coord[0], top_right.coord[1], top_right.coord[2]);

            // If we're at the top of the sphere, use bottom_right for v3 instead
            if (latitude == numLatitude - 1)
                v3 = rd_pointc(bottom_right.coord[0], bottom_right.coord[1], bottom_right.coord[2]);

            // Calculate the polygon normal using these three vertices
            poly_normal = ((v2 - v1) * (v3 - v1)).flipped_vector().normalized();

            // Pass our last vertex in and draw our polygon
            render_poly(bottom_right, true);
        }
    }

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
    // Update our drawing color
    drawRed = color[0];
    drawGreen = color[1];
    drawBlue = color[2];

    // Update our surface color global
    surface_color[0] = color[0];
    surface_color[1] = color[1];
    surface_color[2] = color[2];

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

/// Sets the surface shader global variable based on the string passed in.
int REDirect::rd_surface(const string &shader_type)
{
    if (shader_type == "metal")
        shader = metal_shader;
    else if (shader_type == "plastic")
        shader = plastic_shader;
    else // Default to using matte shader
        shader = matte_shader;

    return RD_OK;
}

/// Adds a point light to the scene with the specified position, color, and intensity.
int REDirect::rd_point_light(const float pos[3], const float color[], float intensity)
{
    // Create the new point light object
    point_light light;
    light.position = rd_pointc(pos[0], pos[1], pos[2]);
    light.redIntensity = color[0] * intensity;
    light.greenIntensity = color[1] * intensity;
    light.blueIntensity = color[2] * intensity;

    // Add the light to the global array of point lights
    point_lights[num_point_lights] = light;
    num_point_lights++;
    return RD_OK;
}

/// Adds a far light to the scene with the specified direction, color, and intensity.
int REDirect::rd_far_light(const float dir[3], const float color[], float intensity)
{
    // Create the new far light object
    far_light light;
    light.direction = rd_vector(dir[0], dir[1], dir[2]);
    light.redIntensity = color[0] * intensity;
    light.greenIntensity = color[1] * intensity;
    light.blueIntensity = color[2] * intensity;

    // Add the light to the global array of far lights
    far_lights[num_far_lights] = light;
    num_far_lights++;
    return RD_OK;
}

/// Creates an ambient light object and stores it into the global
/// ambient light variable.
int REDirect::rd_ambient_light(const float color[], float intensity)
{
    // Create the ambient light struct object
    ambient_light light;
    light.redIntensity = color[0] * intensity;
    light.greenIntensity = color[1] * intensity;
    light.blueIntensity = color[2] * intensity;

    // Store the ambient light into the global variable
    global_ambient_light = light;

    return RD_OK;
}

/// Sets the specular color and specular exponent.
/// @param color An array of three floats with the RGB values to set specular color to.
/// @param exponent The new specular expontent.
int REDirect::rd_specular_color(const float color[], int exponent)
{
    // Update all the global values
    specular_color[0] = color[0];
    specular_color[1] = color[1];
    specular_color[2] = color[2];
    specular_exponent = exponent;
    return RD_OK;
}

/// Updates the global ambient coefficient.
/// @param Ka The new ambient coefficient value.
int REDirect::rd_k_ambient(float Ka)
{
    ambient_coefficient = Ka;
    return RD_OK;
}

/// Updates the global diffuse coefficient.
/// @param Kd The new diffuse coefficient value.
int REDirect::rd_k_diffuse(float Kd)
{
    diffuse_coefficient = Kd;
    return RD_OK;
}

/// Updates the global specular coefficient.
/// @param Ks The new specular coefficient value.
int REDirect::rd_k_specular(float Ks)
{
    specular_coefficient = Ks;
    return RD_OK;
}

/// Calculates the ambient color, dependent on the flags.
/// The calculated color is passed out into the color parameter.
void REDirect::calculate_ambient(float *color)
{
    // Create a variable for calculating the color, defaulted to using the surface color
    float calc_color[3] = {surface_color[0], surface_color[1], surface_color[2]};

    // If interpolation and vertex_color are on, use that instead of the surface_color
    if (interpolation_flag && vertex_color_flag)
    {
        calc_color[0] = surface_point_values.coord[ATTR_R];
        calc_color[1] = surface_point_values.coord[ATTR_G];
        calc_color[2] = surface_point_values.coord[ATTR_B];
    }

    // Calculate the color using the ambient coefficient and current ambient light
    color[0] = ambient_coefficient * calc_color[0] * global_ambient_light.redIntensity;
    color[1] = ambient_coefficient * calc_color[1] * global_ambient_light.greenIntensity;
    color[2] = ambient_coefficient * calc_color[2] * global_ambient_light.blueIntensity;
}

/// Calculates the diffuse color, dependent on the current flags.
/// The calculated color is passed out into the color parameter.
void REDirect::calculate_diffuse(float *color)
{
    // Create a variable for calculating the color, defaulted to using the surface color
    float calc_color[3] = {surface_color[0], surface_color[1], surface_color[2]};

    // If interpolation and vertex_color are on, use that instead of the surface_color
    if (interpolation_flag && vertex_color_flag)
    {
        calc_color[0] = surface_point_values.coord[ATTR_R];
        calc_color[1] = surface_point_values.coord[ATTR_G];
        calc_color[2] = surface_point_values.coord[ATTR_B];
    }

    // Create values for each channel's diffuse sum and to hold the object normal
    float red = 0, green = 0, blue = 0;
    rd_vector normal;

    // If we should interpolate and use vertex normals, store those
    if (interpolation_flag && vertex_normal_flag)
        normal = rd_vector(surface_point_values.coord[ATTR_NX], surface_point_values.coord[ATTR_NY], surface_point_values.coord[ATTR_NZ]).normalized();
    else // Otherwise use the global poly normal
        normal = poly_normal.normalized();

    // Calculate the diffuse for each far light first
    for (int index = 0; index < num_far_lights; index++)
    {
        // Calculate the dot product then apply the intensity from each color channel
        float dot = normal ^ far_lights[index].direction.normalized();
        if (dot <= 0) continue; // Skip any lights that aren't shining onto our object
        red += dot * far_lights[index].redIntensity;
        green += dot * far_lights[index].greenIntensity;
        blue += dot * far_lights[index].blueIntensity;
    }

    // Calculate the diffuse for each point light next
    for (int index = 0; index < num_point_lights; index++)
    {
        // Calculate the light direction for the point light
        rd_vector light_vector = point_lights[index].position - rd_pointc(
            surface_point_values.coord[ATTR_WORLD_X], surface_point_values.coord[ATTR_WORLD_Y], surface_point_values.coord[ATTR_WORLD_Z]);

        // Calculate the intensity falloff
        float fallOff = 1 / (powf(light_vector.magnitude(), 2));
        float redIntensity = point_lights[index].redIntensity * fallOff;
        float greenIntensity = point_lights[index].greenIntensity * fallOff;
        float blueIntensity = point_lights[index].blueIntensity * fallOff;

        // Calculate the dot product then apply the intensity from each color channel
        float dot = normal ^ light_vector.normalized();
        if (dot <= 0) continue;
        red += dot * redIntensity;
        green += dot * greenIntensity;
        blue += dot * blueIntensity;
    }

    // Multiply each channel by the coefficient and surface color
    red *= diffuse_coefficient * calc_color[0];
    green *= diffuse_coefficient * calc_color[1];
    blue *= diffuse_coefficient * calc_color[2];

    // Add our diffuse to the out color and clamp each channel between 0 and 1
    color[0] = clamp(color[0] + red, 0, 1);
    color[1] = clamp(color[1] + green, 0, 1);
    color[2] = clamp(color[2] + blue, 0, 1);
}

/// Calculates the specular contribution of the color passed in
/// using its world position from surface_values to sum over each
/// light in the scene to calculate the final specular contribution
/// at the current pixel.
void REDirect::calculate_specular(float *color)
{
    // Create a variable for calculating the color, defaulted to using the specular colors
    float calc_color[3] = {specular_color[0], specular_color[1], specular_color[2]};

    // If the use_surface flag is on, use that instead of the specular color
    if (use_surface_flag)
    {
        calc_color[0] = surface_color[0];
        calc_color[1] = surface_color[1];
        calc_color[2] = surface_color[2];
    }

    // Create values for each channel's specular sum
    float red = 0, green = 0, blue = 0;

    // Store our normal for this vector
    rd_vector normal;
    if (interpolation_flag && vertex_normal_flag)
        normal = rd_vector(surface_point_values.coord[ATTR_NX], surface_point_values.coord[ATTR_NY], surface_point_values.coord[ATTR_NZ]).normalized();
    else // Otherwise use the global poly normals
        normal = poly_normal.normalized();

    // Calculate the view vector for the surface
    rd_vector view = (camera_eye - rd_pointc(surface_point_values.coord[ATTR_WORLD_X], surface_point_values.coord[ATTR_WORLD_Y], surface_point_values.coord[ATTR_WORLD_Z])).normalized();

    // If view dot normal is <= 0, it's the back of the poly and we wouldn't see reflected light
    if ((view ^ normal) <= 0) return;

    // Calculate the specular for each far light
    for (int index = 0; index < num_far_lights; index++)
    {
        // Calculate the direction of the light and the reflected vector of the light
        rd_vector light = far_lights[index].direction.normalized();
        rd_vector reflected = rd_vector::reflect(light, normal).normalized();

        // Only apply specular to the front of faces, not the back
        if ((normal ^ light) <= 0) continue;

        // Calculate the dot product and intensity for each color channel
        float dot = view ^ reflected;
        if (dot <= 0) continue; // Skip any lights that aren't shining on our surface
        dot = powf(dot, specular_exponent); // Apply our shininess

        red += specular_coefficient * calc_color[0] * far_lights[index].redIntensity * dot;
        green += specular_coefficient * calc_color[1] * far_lights[index].greenIntensity * dot;
        blue += specular_coefficient * calc_color[2] * far_lights[index].blueIntensity * dot;
    }

    // Calculate the specular for each point light next
    for (int index = 0; index < num_point_lights; index++)
    {
        // Calculate the light direction and reflection vector for the point light
        rd_vector light_vector = point_lights[index].position - rd_pointc(
            surface_point_values.coord[ATTR_WORLD_X], surface_point_values.coord[ATTR_WORLD_Y], surface_point_values.coord[ATTR_WORLD_Z]);
        rd_vector reflected = rd_vector::reflect(light_vector, normal).normalized();

        // Make sure we're on the front of a polygon and not the back
        if ((normal ^ light_vector) <= 0) continue;

        // Calculate the intensity falloff
        float fallOff = 1 / (powf(light_vector.magnitude(), 2));
        float redIntensity = point_lights[index].redIntensity * fallOff;
        float greenIntensity = point_lights[index].greenIntensity * fallOff;
        float blueIntensity = point_lights[index].blueIntensity * fallOff;

        // Calculate the dot product for our summation
        float dot = view ^ reflected;
        if (dot <= 0) continue; // Skip any lights that aren't shining on the surface
        dot = powf(dot, specular_exponent);

        // Apply the intensity from each color to the summation
        red += specular_coefficient * calc_color[0] * redIntensity * dot;
        green += specular_coefficient * calc_color[1] * greenIntensity * dot;
        blue += specular_coefficient * calc_color[2] * blueIntensity * dot;
    }

    // Add our specular to the out color and clamp between 0 and 1
    color[0] = clamp(color[0] + red, 0, 1);
    color[1] = clamp(color[1] + green, 0, 1);
    color[2] = clamp(color[2] + blue, 0, 1);
}

/// Updates the color passed in to reflect a matte surface shading.
void REDirect::matte_shader(float *color)
{
    // Calculate the ambient and diffuse (no specular)
    calculate_ambient(color);
    calculate_diffuse(color);
}

/// Updates the color passed in to reflect a metal surface shading.
void REDirect::metal_shader(float *color)
{
    // Make sure to use the surface color for the metallic look
    use_surface_flag = true;

    // Calculate the ambient and specular (no diffuse)
    calculate_ambient(color);
    calculate_specular(color);
}

/// Updates the color passed in to reflect a more plastic
/// style of surface shading. The outcome is heavily
/// dependent on global values though.
void REDirect::plastic_shader(float *color)
{
    // Don't use surface color, not metallic here
    use_surface_flag = false;

    // Calculate all three values
    calculate_ambient(color);
    calculate_diffuse(color);
    calculate_specular(color);
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

/// This function checks the z-buffer to see if the provided point
/// should be drawn or not and draws it and updates the z-buffer if
/// it should in fact be drawn.
/// @param point The cartesian point to draw.
void REDirect::plot_pixel(rd_pointc point, float* color = nullptr)
{
    // If color is nullptr, set it to the global drawColor
    if (color == nullptr)
        color = new float[3] {drawRed, drawGreen, drawBlue};

    // Store out our x and y from the point
    int x = (int)point.get_x();
    int y = (int)point.get_y();

    // If the x or the y are on the very edge, subtract one
    if (x == display_xSize) x -= 1;
    if (y == display_ySize) y -= 1;

    // Check if the z-value of the point is larger than the point in the buffer
    if (point.get_z() < depth_buffer[y][x])
    {
        rd_write_pixel((int)point.get_x(), (int)point.get_y(), color);
        depth_buffer[y][x] = point.get_z();
    }
}

/// A function that calculates the world to clip transformation matrix and
/// stores it our into our global world to clip matrix variable.
void REDirect::calculate_world_to_clip()
{
    // Create our camera vectors to create the view matrix
    rd_vector world_up(camera_up);
    rd_vector forward = (camera_eye - camera_at).normalized();
    rd_vector right = (forward * world_up).normalized();
    rd_vector up = (right * forward).normalized();

    // Create the view matrix using the forward, right, and up vectors from the camera
    rd_xform view_matrix = {
        right.GetX(), right.GetY(), right.GetZ(), -(right ^ camera_eye),
        up.GetX(), up.GetY(), up.GetZ(), -(up ^ camera_eye),
        forward.GetX(), forward.GetY(), forward.GetZ(), -(forward ^ camera_eye),
        0, 0, 0, 1
    };

    // Create our perspective transform matrix
    float fov_scale = tanf((camera_fov / 2) * (M_PI/180));
    float aspect_ratio = (float)display_xSize / (float)display_ySize;
    rd_xform perspective_matrix = {
        1/(2 * aspect_ratio * fov_scale), 0, 0.5, 0,
        0, 1/(2 * fov_scale), 0.5, 0,
        0, 0, far_clip/(far_clip - near_clip), (-far_clip * near_clip)/(far_clip - near_clip),
        0, 0, 1, 0
    };

    // Store our final world to clip matrix by finding the cross product perspective x view
    world_to_clip = perspective_matrix * view_matrix;
}

/// A function that calculates the clip to device transformation matrix
/// and stores it directly into our global matrix variable
void REDirect::calculate_clip_to_device()
{
    // Tiny epsilon to subtract so our things don't go off-screen
    float epsilon = 0.001;

    // Create our clip to device matrix and store it into our global variable
    clip_to_device = {
        ((float)display_xSize - epsilon), 0, 0, 0,
        0, -((float)display_ySize - epsilon), 0, ((float)display_ySize - epsilon),
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

/// Runs the provided point through the point pipeline, converting it from
/// world to clip coordinates, checking if it should be clipped or not, and then
/// converting it from clip to device before drawing it to the screen.
/// @param point The homogeneous point to render or clip.
void REDirect::render_point(rd_pointh point)
{
    // Transform our point by the current transform
    point = current_transform * point;

    // Run our point through the world->clip pipeline
    point = world_to_clip * point;

    // Check if we should clip the point or render it
    if (check_point_clip(point)) // We can exit here as we don't need to render this point
        return;

    // Perform the perspective divide on our point
    float w = point.get_w();
    point = rd_pointh(point.get_x() / w, point.get_y() / w, point.get_z() / w, point.get_w() / w);

    // Run our point through the clip->device pipeline
    point = clip_to_device * point;

    // Write our pixel to the screen after having run it through the pipeline
    plot_pixel(rd_pointc(point));
}

/// Creates boundary coordinates from Brinn's clipping algorithm and if any of the
/// boundary coordinates are negative, returns true to clip the point.
/// @param point A homogeneous point to check whether it is in bounds.
/// @returns True if the point should be clipped and false if not.
bool REDirect::check_point_clip(rd_pointh point)
{
    // Create a simple boundary coordinate array
    float boundary_coordinates[6] = {
        point.get_x(), point.get_w() - point.get_x(),
        point.get_y(), point.get_w() - point.get_y(),
        point.get_z(), point.get_w() - point.get_z()
    };

    // Traverse the boundary coordinates and return true to clip the point if any coord is negative
    for (int index = 0; index < 6; index++)
        if (boundary_coordinates[index] < 0) return true;

    // Return false, we should not clip this point, if we made it past the for loop
    return false;
}

/// Runs the provided point through the current transform, then
/// through our world to clip transform matrix, then passes the
/// new point and our draw flag to the clip_line function.
/// @param point The homogeneous point we want to plot on a line.
/// @param should_draw Whether a line should be drawn between this vertex and the last.
void REDirect::render_line(rd_pointh point, bool should_draw)
{
    // Run our point through our current transformations
    point = current_transform * point;

    // Run our point through the world->clip transformation
    point = world_to_clip * point;

    // Send the point to the line clipping routine next
    clip_line(point, should_draw);
}

/// Using Blinn's clipping algorithm, uses boundary coordinates
/// and kodes to detect lines rendering outside of screen space and
/// for lines that are not fully outside of clipping planes, calculates
/// new starting and ending points to draw the line with that are within
/// the clipping planes of the current scene.
/// @param point The homogenous point to perform clipping on. This point
///             should be in clipping space.
/// @param should_draw Whether a line should be drawn between this vertex and the last,
///             or if the last_vertex should just be updated / moved to this point.
void REDirect::clip_line(rd_pointh point, bool should_draw)
{
    // Static variables to hold onto the last boundary coords and kode
    static float lastBoundaryCoords[6];
    static int lastKode;

    // Create our boundary coordinate from the homogenous point
    float boundaryCoords[6] = {
        point.get_x(), point.get_w() - point.get_x(),
        point.get_y(), point.get_w() - point.get_y(),
        point.get_z(), point.get_w() - point.get_z()
    };

    // Create the kode and mask
    int kode = 0, mask = 1;

    // Step through the boundary coordinates to create the kode
    for (int index = 0; index < 6; index++)
    {
        // Update the bit in our kode if the coordinate is negative
        if (boundaryCoords[index] < 0)
            kode |= mask;

        // Shift the mask by one bit left
        mask <<= 1;
    }

    // If we should draw, check for a trivial reject and trivial accept
    if (should_draw && (lastKode & kode) == 0) // If this fails it's a trivial reject on this line
    {
        // Trivial accept, we can simply draw from here
        if ((lastKode | kode) == 0)
            plot_line(point);

        // Otherwise we calculate clipping
        else
        {
            // Create our "super" kode to use
            int superKode = lastKode | kode;

            // Create our alphaMin and alphaMax
            float alphaMin = 0, alphaMax = 1;

            // Step through each bit in the kode to see if we should calculate an alpha
            int mask = 1; // Use a mask to check the kodes
            for (int index = 0; index < 6; index++)
            {
                // If the superKode is 0 at this bit, shift the mask and move on
                if ((superKode & mask) == 0)
                {
                    mask <<= 1;
                    continue;
                }

                // Find the alpha for our current position
                float alpha = lastBoundaryCoords[index] / (lastBoundaryCoords[index] - boundaryCoords[index]);

                // If lastKode at this bit is 1, we are moving inside to out so adjust our min
                if ((lastKode & mask) != 0)
                    alphaMin = std::max(alphaMin, alpha);
                else // Otherwise we are moving outside to in so adjust our max
                    alphaMax = std::min(alphaMax, alpha);

                // Shift the mask
                mask <<= 1;
            }

            // Only proceed with drawing if we did not clip the line out of existence
            if (!(alphaMin >= alphaMax))
            {
                // Using our alphas, create our updated points
                rd_pointh start_point = last_vertex + ((point - last_vertex) * alphaMin);
                rd_pointh end_point = last_vertex + ((point - last_vertex) * alphaMax);
                last_vertex = start_point; // Update last_vertex for drawing now that our calculations are done

                // Draw with our clipped lines
                plot_line(end_point);
            }
        }
    }

    // Update our global static variables
    last_vertex = point;
    lastKode = kode;
    for (int index = 0; index < 6; index++)
        lastBoundaryCoords[index] = boundaryCoords[index];
}

/// Uses the DDA line drawing algorithm to draw a line on the screen from
/// last_vertex (our starting point) to the passed in point (our ending point).
/// This function also calculates the change in z for use with depth buffering.
/// Each calculated point is passed to plot_pixel for it to be drawn on the
/// screen with proper buffering.
/// @param point The homogenous point to convert to device coordinates and draw towards.
void REDirect::plot_line(rd_pointh point)
{
    // Convert the last vertex and this vertex to device coordinates
    rd_pointc start_vertex = rd_pointc(clip_to_device * last_vertex);
    rd_pointc end_vertex = rd_pointc(clip_to_device * point);

    // Calculate dX and dY, the difference in endpoints
    int dX = abs((int)start_vertex.get_x() - (int)end_vertex.get_x());
    int dY = abs((int)start_vertex.get_y() - (int)end_vertex.get_y());

    // Store our number of steps based on the max between dX and dY
    const int NSTEPS = std::max(dX, dY);
    if (NSTEPS <= 0) return; // We can skip the next calculations if we don't have any steps to take

    // Set our initial variables based on the start vertex
    float x = start_vertex.get_x();
    float y = start_vertex.get_y();
    float z = start_vertex.get_z();

    // Set our d variables
    float dx = (end_vertex.get_x() - x) / NSTEPS;
    float dy = (end_vertex.get_y() - y) / NSTEPS;
    float dz = (end_vertex.get_z() - z) / NSTEPS;

    // Draw our line
    for (int index = 0; index < NSTEPS; index++)
    {
        x = start_vertex.get_x() + index * dx;
        y = start_vertex.get_y() + index * dy;
        z = start_vertex.get_z() + index * dz;

        // Create our new point from these values
        rd_pointc new_point = rd_pointc(x, y, z);

        // Plot our pixel on the screen at the x and y
        plot_pixel(new_point);
    }
}

/// This function acts as the entry point into the polygon pipeline, taking an attributed point
/// and sending it all the way down through clipping and scan conversion to draw based on the
/// vertices stored in the internal vertex_list at the time of a draw call.
/// @param point The attributed point to add to the pipeline.
/// @param should_draw Whether we should render a full polygon from this point to the rest currently stored.
/// @return 0 if the polygon renders okay, and -1 if an error occurs.
int REDirect::render_poly(rd_pointa point, bool should_draw)
{
    // Variables for working with points easier
    rd_pointh geometry, dev;
    rd_vector normal;

    // Set up some static variables for keeping track of current vertices
    const int MAX_VERTEX_LIST_SIZE = 50;
    static rd_pointa vertex_list[MAX_VERTEX_LIST_SIZE];
    static rd_pointa clipped_list[MAX_VERTEX_LIST_SIZE];
    static int num_vertex = 0;

    // Run geometry through current transform
    geometry[0] = point.coord[0];
    geometry[1] = point.coord[1];
    geometry[2] = point.coord[2];
    geometry[3] = point.coord[3];
    geometry = current_transform * geometry;

    // Save the values into the attributed point's world coordinates
    point.coord[ATTR_WORLD_X] = geometry[0];
    point.coord[ATTR_WORLD_Y] = geometry[1];
    point.coord[ATTR_WORLD_Z] = geometry[2];

    // Convert the normal to world coordinates as well
    normal[0] = point.coord[ATTR_NX];
    normal[1] = point.coord[ATTR_NY];
    normal[2] = point.coord[ATTR_NZ];
    normal = normal * normal_transform;

    // Put the normal values back into the point in world coordinates
    point.coord[ATTR_NX] = normal[0];
    point.coord[ATTR_NY] = normal[1];
    point.coord[ATTR_NZ] = normal[2];
    point.coord[ATTR_CONSTANT] = 1;

    // Convert the geometry to clipping coordinates
    geometry = world_to_clip * geometry;

    // Put the transformed vertex back into the attributed point
    point.coord[0] = geometry[0];
    point.coord[1] = geometry[1];
    point.coord[2] = geometry[2];
    point.coord[3] = geometry[3];

    // Store in vertex list
    if (num_vertex == MAX_VERTEX_LIST_SIZE)
        return -1; // Too many vertices, overflow

    vertex_list[num_vertex] = point;
    num_vertex++;

    // Move along to the next vertex if we shouldn't draw
    if (!should_draw)
        return 0;

    // Otherwise pass our info down to poly_clip -- continue if there is something to draw
    if ((num_vertex = clip_poly(num_vertex, vertex_list, clipped_list)))
    {
        // Pre-process the vertex list
        for (int index = 0; index < num_vertex; index++)
        {
            // Convert the geometry to device coordinates
            dev[0] = clipped_list[index].coord[0];
            dev[1] = clipped_list[index].coord[1];
            dev[2] = clipped_list[index].coord[2];
            dev[3] = clipped_list[index].coord[3];\
            dev = clip_to_device * dev;

            // Put the X and Y back in as they're the only ones changed meaningfully
            clipped_list[index].coord[0] = dev[0];
            clipped_list[index].coord[1] = dev[1];

            // Perform the perspective divide
            clipped_list[index].coord[0] /= clipped_list[index].coord[3];
            clipped_list[index].coord[1] /= clipped_list[index].coord[3];
            clipped_list[index].coord[2] /= clipped_list[index].coord[3];

            // Normalize from the constant onwards
            for (int i = 4; i < ATTR_SIZE; i++)
                clipped_list[index].coord[i] /= clipped_list[index].coord[3];
        }

        // Convert the poly_normal global into world coordinates
        poly_normal = poly_normal * normal_transform;

        // Pass the clipped geometry into the scan conversion function
        draw_poly(num_vertex, clipped_list);
    }

    // Reset our vertex list for the next polygon
    num_vertex = 0;
    return 0;
}

/// This function takes clipped vertices and builds an edge table with
/// them then begins scan conversion stepping through each line on
/// the display and filling between any edges.
/// @param num_vertex The number of vertices in the attributed point array.
/// @param clipped_list An array of clipped attributed points.
void REDirect::draw_poly(int num_vertex, rd_pointa *clipped_list)
{
    // Head of active edge table
    rd_edge* active_edge_table;

    // Return if we don't cross any scan lines
    if (!build_edge_list(num_vertex, clipped_list))
        return;

    // Clear the active edge table
    active_edge_table = new rd_edge;

    // Scan over the scanlines of the display
    for (int scanline = 0; scanline < display_ySize; scanline++)
    {
        // Take the edge starting on this scanline and add it to the AET
        add_active_list(scanline, active_edge_table);

        // Draw if the AET is not empty on this scanline
        if (active_edge_table->next != nullptr)
        {
            fill_between_edges(scanline, active_edge_table);
            update_aet(scanline, active_edge_table);
            resort_aet(active_edge_table);
        }
    }

    // Delete the last part of the AET
    delete active_edge_table;
}

/// Builds the edge list with the vertices by iterating through
/// the vertices and checking for scanline crossings, creating
/// edges from vertices that form an edge that crosses the scanlines.
/// @param num_vertex Number of vertices in the attributed point array.
/// @param points Attributed points to build the edge list from.
/// @return True if at least one scanline is crossed and false if not.
bool REDirect::build_edge_list(int num_vertex, rd_pointa *points)
{
    // The index of the trailing vertex of our edge
    int v1 = num_vertex - 1;

    // Create our return value defaulted to false
    bool scanline_crossed = false;

    // Loop over the leading vertices in the polygon
    for (int v2 = 0; v2 < num_vertex; v2++)
    {
        // If v1 and v2 are on different y levels, set scanline_crossed to true
        if ((int)points[v1].coord[1] != (int)points[v2].coord[1])
        {
            scanline_crossed = true;

            // Make an edge record from v1 -> v2 if v1 is smaller
            if (points[v1].coord[1] < points[v2].coord[1])
                make_edge_record(points[v1], points[v2]);
            else
                make_edge_record(points[v2], points[v1]);
        }

        // Move to the next edge
        v1 = v2;
    }

    return scanline_crossed;
}

/// Creates a new edge between the provided vertices and inserts
/// them into the edge table at ceil(lower.y).
/// @param lower The attributed point with the lower y-value.
/// @param upper The attributed point with the higher y-value.
void REDirect::make_edge_record(rd_pointa lower, rd_pointa upper)
{
    // Create the increment for the new edge
    float dy = upper.coord[1] - lower.coord[1];

    // Create the pointer for the new edge
    rd_edge* edge = new rd_edge();
    edge->increment = (upper - lower) / dy;

    // Ensure the edge starts on a scanline
    float factor = ceilf(lower.coord[1]) - lower.coord[1]; // Fractional pos of first scanline given here

    // Calculate the starting values for the edge
    edge->point = lower + (factor * edge->increment);

    // Find the last scanline for the edge and insert it into the edge table list
    edge->yLast = (ceilf(upper.coord[1]) - 1);
    insert_edge(&edge_table[(int)ceilf(lower.coord[1])], edge);
}

/// Adds all edges at the specified scanline to the Active Edge Table
/// and clears the normal edge table at that scanline.
/// @param scanline The scanline number to make the active list from.
/// @param aet Pointer to the head of the Active Edge Table.
void REDirect::add_active_list(int scanline, rd_edge *aet)
{
    // Get the edges starting on the scanline
    rd_edge* next = edge_table[scanline].next;

    // Move through the list and insert each entry into the active edge table
    rd_edge* held_list;
    while (next)
    {
        held_list = next->next; // Hold the rest of the list
        insert_edge(aet, next);
        next = held_list;
    }

    // Keep the edge table clean -- edges have been transferred
    edge_table[scanline].next = nullptr;
}

/// Adds the specified edge to the edge list sorted by ascending
/// x-coordinate values.
/// @param list The head of the edge list to add the edge to.
/// @param edge The edge to add to the edge list.
void REDirect::insert_edge(rd_edge *list, rd_edge *edge)
{
    // Create two pointers to sort the edges by x-coordinate values
    rd_edge* p, *q = list;

    // p leads the list
    p = q->next;
    while (p != nullptr && (edge->point.coord[0] > p->point.coord[0]))
    {
        // Step to the next edge
        q = p;
        p = p->next;
    }

    // Link the edge into the list after q
    edge->next = q->next;
    q->next = edge;
}

/// Update the nodes in the Active Edge Table, either
/// incrementing their values or deleting dead edges as
/// we reach the end of the edge's span.
/// @param scanline Scan line to check against yLast.
/// @param aet Pointer to the head of the Active Edge Table.
void REDirect::update_aet(int scanline, rd_edge *aet)
{
    // Keep track of our active and next active edges
    rd_edge* active = aet, *next = aet->next;

    // Iterate through each edge in the list
    while (next)
    {
        // Check if we're on the last scanline of the edge
        if (scanline == next->yLast)
        {
            // If yes, move p along and get rid of tail node
            next = next->next;
            delete_next_edge(active);
        }
        else // If not, update attributed values
        {
            next->point = next->point + next->increment;
            active = next;
            next = next->next;
        }
    }
}

/// Deletes the edge immediately proceeding the passed-in edge,
/// shifting pointers around so as not to break the list.
/// @param edge The edge to delete the next edge of.
void REDirect::delete_next_edge(rd_edge *edge)
{
    rd_edge* deletee = edge->next;
    edge->next = deletee->next;
    delete deletee;
}

/// Re-sorts the Active Edge Table by reinserting each edge
/// to ensure proper sorting in x-ascending order.
/// @param aet Pointer to the head of the Active Edge Table.
void REDirect::resort_aet(rd_edge *aet)
{
    // Hold temporary pointers to edges for moving them around
    rd_edge* q, *p = aet->next;

    // Resort the table by increasing x-values
    aet->next = nullptr;
    while (p)
    {
        q = p->next;
        insert_edge(aet, p);
        p = q;
    }
}

/// This function takes each edge pair from the Active Edge Table
/// and fills in the area between them while incrementing all
/// the attributed values as needed.
/// @param scanline The scan line to fill, used as the y-value.
/// @param aet Pointer to the head of the Active Edge Table.
void REDirect::fill_between_edges(int scanline, rd_edge *aet)
{
    rd_edge *current = aet->next, *next;
    while (current)
    {
        // Figure out why certain specific cases have stray edges
        if (current->next == nullptr) break;

        // Get the pair of edges from the active edge table
        next = current->next;

        // If they are not at the same x, calculate the increment and draw
        if (current->point.coord[0] != next->point.coord[0])
        {
            // Calculate the attribute increments along the scanline
            float dx = next->point.coord[0] - current->point.coord[0];
            rd_pointa increment = (next->point - current->point) / dx;

            // Find the starting values for the edge
            float factor = ceilf(current->point.coord[0]) - current->point.coord[0]; // Fractional pos for first pixel crossing
            rd_pointa value = current->point + factor * increment;
            float end_x = ceilf(next->point.coord[0]);

            // While not at the end x, plot each pixel along the way
            while (value.coord[0] < end_x)
            {
                // Store the values into surface_point_values and divide by the constant
                surface_point_values = value;
                for (int index = 5; index < ATTR_SIZE; index++)
                    surface_point_values.coord[index] /= surface_point_values.coord[ATTR_CONSTANT];

                // Get the color from the current shader
                float* color = new float[3];
                shader(color);

                rd_pointc point(value.coord[0], scanline, value.coord[2]);
                plot_pixel(point, color);

                // Increment the values
                value = value + increment;
            }
        }

        // Move to the next edge pair
        current = next->next;
    }
}

/// This function acts as the entry point to the polygon clipping algorithm
/// heavily based on the Sutherland-Hodgman algorithm. It sets up the
/// seen and flag arrays and begins recursion, while also handling the
/// last vertices gracefully.
/// @param num_vertex The number of vertices in the vertex list.
/// @param vertex_list An array of vertices to run through clipping.
/// @param clipped_list An array to pass out the clipped vertices to.
/// @return The number of vertices inside clipped_list.
int REDirect::clip_poly(int num_vertex, rd_pointa *vertex_list, rd_pointa *clipped_list)
{
    // Create our attributed point and flag arrays
    rd_pointa first_seen[6], last_seen[6];
    bool stage_seen[6];

    // Create our count of vertices to return at the end
    int vertex_count = 0;

    // Initialize our flag array to false
    for (int index = 0; index < 6; index++)
        stage_seen[index] = false;

    // Loop through all of our vertices and pass them into the recursive clipper
    for (int index = 0; index < num_vertex; index++)
        clip_vertex(vertex_list[index], 0, first_seen, last_seen, stage_seen, clipped_list, &vertex_count);

    // Recursively handle the edge between first and last seen at each stage.
    clip_last_vertex(first_seen, last_seen, stage_seen, clipped_list, &vertex_count);
    return vertex_count;
}

/// This function passes the vertex through each boundary and calculates its
/// intersection using something akin to Blinn's clipping algorithm. Points that
/// don't cross boundaries are simply passed on or placed into the clipped_list if at the end.
/// @param vertex The vertex to clip.
/// @param boundary The boundary to clip the vertex on (0-5).
/// @param first_array Array of attributed points that contain the first seen point for each boundary.
/// @param last_array Array of attributed points that contain the last seen point for each boundary.
/// @param flag_array Array of bools that say whether this boundary has been visited yet.
/// @param clipped_list Array of attributed points to output clipped vertices into.
/// @param vertex_count Pointer to the number of vertices placed inside clipped_list.
void REDirect::clip_vertex(rd_pointa vertex, int boundary, rd_pointa *first_array, rd_pointa *last_array, bool *flag_array, rd_pointa* clipped_list, int *vertex_count)
{
    // Check if this is the first time a point has been seen by this stage
    if (!flag_array[boundary]) // If yes, set this as the first point and toggle the flag
    {
        first_array[boundary] = vertex;
        flag_array[boundary] = true;
    }
    else // Otherwise a previous point exists
    {
        // Check edge from last to current
        if (crosses_boundary(last_array[boundary], vertex, boundary))
        {
            rd_pointa intersection = boundary_intersection(vertex, last_array[boundary], boundary);
            if (boundary == 5) // If we're at the last boundary index output the point
            {
                clipped_list[*vertex_count] = intersection;
                (*vertex_count)++;
            }
            else // Otherwise send it recursively to the next stage
                clip_vertex(intersection, boundary + 1, first_array, last_array, flag_array, clipped_list, vertex_count);
        }
    }

    // Save the most recent vertex seen at this edge
    last_array[boundary] = vertex;

    // If the vertex is inside the boundary, pass it along or put it into clipped_list
    if (inside_boundary(vertex, boundary))
    {
        if (boundary == 5) // If we're at the last boundary index output the point
        {
            clipped_list[*vertex_count] = vertex;
            (*vertex_count)++;
        }
        else // Otherwise send it recursively to the next stage
            clip_vertex(vertex, boundary + 1, first_array, last_array, flag_array, clipped_list, vertex_count);
    }
}

/// This function handles the special case of the edge formed between the first and last
/// vertex seen at each boundary. It will only pass it through recursively if it crosses
/// a boundary. Otherwise it will be left alone.
/// @param first_array Array of attributed points that contain the first seen point for each boundary.
/// @param last_array Array of attributed points that contain the last seen point for each boundary.
/// @param flag_array Array of bools that say whether this boundary has been visited yet.
/// @param clipped_list Array of attributed points to output clipped vertices into.
/// @param vertex_count Pointer to the number of vertices placed inside clipped_list.
void REDirect::clip_last_vertex(rd_pointa *first_array, rd_pointa *last_array, bool *flag_array, rd_pointa *clipped_list, int *vertex_count)
{
    // Iterate through each boundary stage
    for (int index = 0; index < 6; index++)
    {
        // If this boundary wasn't used skip it
        if (!flag_array[index]) continue;

        // Only handle cases that intersect the boundary
        if (crosses_boundary(last_array[index], first_array[index], index))
        {
            rd_pointa intersection = boundary_intersection(last_array[index], first_array[index], index);
            if (index == 5) // If we're at the last boundary index output the point
            {
                clipped_list[*vertex_count] = intersection;
                (*vertex_count)++;
            }
            else // Otherwise send it recursively to the next stage
                clip_vertex(intersection, index + 1, first_array, last_array, flag_array, clipped_list, vertex_count);
        }
    }
}

/// Using the clipping planes from Blinn's algorithm, checks if
/// the point is inside the specified boundary or not.
/// @param point The attributed point to check.
/// @param boundary The boundary to check against.
/// @return True if the point is within the boundary and false if not.
bool REDirect::inside_boundary(rd_pointa point, int boundary)
{
    // Use a switch to check the specified boundary
    switch (boundary)
    {
        case 0: // left boundary
            return point.coord[0] >= 0;
        case 1: // right boundary
            return point.coord[3] - point.coord[0] >= 0;
        case 2: // bottom boundary
            return point.coord[1] >= 0;
        case 3: // top boundary
            return point.coord[3] - point.coord[1] >= 0;
        case 4: // front boundary
            return point.coord[2] >= 0;
        case 5: // back boundary
            return point.coord[3] - point.coord[2] >= 0;
    }

    // Return false if somehow an invalid boundary was used
    return false;
}

/// Checks if each point is inside the specified boundary and
/// if one is and one is not then they cross the boundary.
/// @param v1 The first vertex on the edge.
/// @param v2 The second vertex on the edge.
/// @param boundary The boundary to check against.
/// @return True if the edge formed by the two points cross the boundary, else false.
bool REDirect::crosses_boundary(rd_pointa v1, rd_pointa v2, int boundary)
{
    return inside_boundary(v1, boundary) != inside_boundary(v2, boundary);
}

/// Finds where the edge between v1 and v2 intersect the given boundary
/// and returns that as an interpolated attributed point.
/// @param v1 The first vertex on the edge.
/// @param v2 The second vertex on the edge.
/// @param boundary The boundary to find intersection against.
/// @return The attributed point where the edge intersects with the boundary.
rd_pointa REDirect::boundary_intersection(rd_pointa v1, rd_pointa v2, int boundary)
{
    // Use a switch to calculate alpha based on our boundary
    float alpha = 0;
    switch (boundary)
    {
        case 0: // left boundary (x)
            alpha = v1.coord[0] / (v1.coord[0] - v2.coord[0]);
            break;
        case 1: // right boundary (w-x)
            alpha = (v1.coord[3] - v1.coord[0]) / ((v1.coord[3] - v1.coord[0]) - (v2.coord[3] - v2.coord[0]));
            break;
        case 2: // bottom boundary (y)
            alpha = v1.coord[1] / (v1.coord[1] - v2.coord[1]);
            break;
        case 3: // top boundary (w-y)
            alpha = (v1.coord[3] - v1.coord[1]) / ((v1.coord[3] - v1.coord[1]) - (v2.coord[3] - v2.coord[1]));
            break;
        case 4: // front boundary (z)
            alpha = v1.coord[2] / (v1.coord[2] - v2.coord[2]);
            break;
        case 5: // back boundary (w-z)
            alpha = (v1.coord[3] - v1.coord[2]) / ((v1.coord[3] - v1.coord[2]) - (v2.coord[3] - v2.coord[2]));
            break;
    }

    // Calculate the interpolation and return that
    rd_pointa interpolated;
    for (int index = 0; index < ATTR_SIZE; index++)
        interpolated.coord[index] = v1.coord[index] + alpha * (v2.coord[index] - v1.coord[index]);

    return interpolated;
}

/// Passed the points into the poly pipeline. I just wrote this so I didn't
/// have to copy paste render_poly and true/false 1000000 times.
void REDirect::draw_face(rd_pointa v1, rd_pointa v2, rd_pointa v3, rd_pointa v4)
{
    // Pass each point in
    render_poly(v1, false);
    render_poly(v2, false);
    render_poly(v3, false);
    render_poly(v4, true);
}

/// Checks for certain special options and toggles them on
/// or off based on the flag passed in as a parameter.
/// @param name A const string reference with the name of the option to change.
/// @param flag The value to set the option to, true or false.
int REDirect::rd_option_bool(const string &name, bool flag)
{
    if (name == "Interpolate") // Update the global interpolation flag
        interpolation_flag = flag;

    return RD_OK;
}

/// Clamps the value within the range.
/// @param val The value to clamp.
/// @param min The lowest value allowed for val.
/// @param max The highest value allowed for val.
/// @returns val within the specified range.
float REDirect::clamp(float val, float min, float max)
{
    // Return min or max if val is outside of their range
    if (val < min) return min;
    if (val > max) return max;

    // Otherwise return val as is
    return val;
}
