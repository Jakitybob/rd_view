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
    // Reset the current transform global variable to an identity matrix
    current_transform = rd_xform();

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

    // Multiply this rotation matrix by our current transform and store it back into the global transform
    current_transform = current_transform * rotation;

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

    // Multiply this rotation matrix by our current transform and store it back into the global transform
    current_transform = current_transform * rotation;

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

    // Multiply this rotation matrix by our current transform and store it back into the global transform
    current_transform = current_transform * rotation;

    return RD_OK;
}

/// Pushes the current transform onto the top of the
/// transformation stack.
int REDirect::rd_xform_push()
{
    // Push the current transform onto our transform stack
    xform_stack.push(current_transform);

    return RD_OK;
}

/// Sets the current transform to whatever is on top
/// of the transformation stack, then pops the top
/// of the stack off.
int REDirect::rd_xform_pop()
{
    // Pop the transform from the top of the stack and set it as our current transform
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

///
/// @param vertex_type UNUSED.
/// @param nvertex The number of vertices in the array.
/// @param vertex A reference to an array of floats that contain all vertex information.
/// @param nface The number of faces
/// @param face
int REDirect::rd_polyset(const string &vertex_type, int nvertex, const vector<float> &vertex, int nface, const vector<int> &face)
{
    // Store out the first vertex of a plane to return back to
    rd_pointh* first_vertex = nullptr;

    // Loop through each face to draw each line in the connected set.
    for (int index = 0; index < face.size(); index++)
    {
        // Store the vertex we should draw from the face array
        int vertex_num = face[index];

        //std::cout << "Vertex num: " << vertex_num << std::endl;

        // If the vertex number is -1, draw the final line then reset first_vertex
        if (vertex_num == -1)
        {
            render_line(*first_vertex, true);
            delete first_vertex; // Free up the last point made
            first_vertex = nullptr;
            continue;
        }

        // Create a point at our vertex
        rd_pointh* point = new rd_pointh(vertex[0 + vertex_num * 3], vertex[1 + vertex_num * 3],  vertex[2 + vertex_num * 3]);

        // If first_vertex is nullptr, set it to this vertex and move there without drawing
        if (first_vertex == nullptr)
        {
            first_vertex = point;
            render_line(*point, false);
        }
        else
            render_line(*point, true);
    }

    // Free up memory and return
    delete first_vertex;
    return RD_OK;
}

/// Draws a cone with the base centered on the XY plane and the
/// tip being height away from the origin at (0, 0, height).
/// @param height The height of the cone.
/// @param radius The radius of the base of the cone.
/// @param thetamax UNUSED.
int REDirect::rd_cone(float height, float radius, float thetamax)
{
    // Create our angle to use while creating segments
    float angle = 0; // In radians

    // Draw each segment's face along the edge of the circle at the base
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        // Put the initial point of the face into the pipeline but don't draw anything yet
        render_line(rd_pointh(radius * cosf(angle), radius * sinf(angle), 0), false);

        // Update the angle to the next step
        angle = index * 2 * M_PI/NUM_SEGMENTS;

        // Plot the line of the base and the line to the tip of the cone
        render_line(rd_pointh(radius * cosf(angle), radius * sinf(angle), 0), true);
        render_line(rd_pointh(0, 0, height), true);
    }

    return RD_OK;
}

/// Draws a cube centered about the origin with a length and height of one on each
/// side. Each line of the cube is passed into the line pipeline where it will be
/// transformed and sent through the world -> device pipeline.
int REDirect::rd_cube()
{
    // Draw the bottom face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, -1, 1), false);
    render_poly(rd_pointa(-1, -1, -1, 1), true);

    // Draw the back face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, -1, 1), false);
    render_poly(rd_pointa(-1, -1, -1, 1), true);

    // Draw the right face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, -1, 1), false);
    render_poly(rd_pointa(-1, -1, -1, 1), true);

    // Draw the front face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, 1, 1, 1), false);
    render_poly(rd_pointa(-1, 1, 1, 1), true);

    // Draw the left face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, -1, 1), false);
    render_poly(rd_pointa(-1, -1, -1, 1), true);

    // Draw the top face of the cube
    render_poly(rd_pointa(-1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, 1, 1), false);
    render_poly(rd_pointa(1, -1, -1, 1), false);
    render_poly(rd_pointa(-1, -1, -1, 1), true);

    return RD_OK;
}

/// Draws a cylinder of the given radius with height zmax - zmin.
/// @param radius The radius of the cylinder.
/// @param zmin The z-coordinate of the bottom of the cylinder.
/// @param zmax The z-coordinate of the top of the cylinder.
/// @param thetamax UNUSED.
int REDirect::rd_cylinder(float radius, float zmin, float zmax, float thetamax)
{
    // Create our angle to use while creating segments
    float angle = 0; // In radians

    // Draw each segment's face along the edge of the circles
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        // Calculate the end point of this face at the top of the cylinder
        rd_pointh end_point(radius * cosf(angle), radius * sinf(angle), zmax);

        // Put the initial point of the face into the pipeline but don't draw anything yet
        render_line(rd_pointh(radius * cosf(angle), radius * sinf(angle), zmin), false);

        // Update the angle to the next step
        angle = index * 2 * M_PI/NUM_SEGMENTS;

        // Plot the three edges of the rectangle
        render_line(rd_pointh(radius * cosf(angle), radius * sinf(angle), zmin), true);
        render_line(rd_pointh(radius * cosf(angle), radius * sinf(angle), zmax), true);
        render_line(end_point, true);
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
    render_circle(radius, height);
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
    // Number of latitudinal and longitudinal slices to draw
    int numLatitude = 10;
    int numLongitude = 12;

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

            // Create the four points
            rd_pointc bottomLeft(radius * cosf(theta) * cosf(phi), radius * cosf(theta) * sinf(phi), radius * sinf(theta));
            rd_pointc topLeft(radius * cosf(thetaNext) * cosf(phi), radius * cosf(thetaNext) * sinf(phi), radius * sinf(thetaNext));
            rd_pointc bottomRight(radius * cosf(theta) * cosf(phiNext), radius * cosf(theta) * sinf(phiNext), radius * sinf(theta));
            rd_pointc topRight(radius * cosf(thetaNext) * cosf(phiNext), radius * cosf(thetaNext) * sinf(phiNext), radius * sinf(thetaNext));

            // Pass the four points in the line drawing routine to create a polygon
            render_line(bottomLeft, false);
            render_line(bottomRight, true);
            render_line(topRight, true);
            render_line(topLeft, true);
            render_line(bottomLeft, true);
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
        if (boundaryCoords[index] < -1e-5f)
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

///
/// @param point The attributed point to add to the pipeline.
/// @param should_draw Whether we should render a full polygon from this point to the rest currently stored.
int REDirect::render_poly(rd_pointa point, bool should_draw)
{
    // Variables for working with points easier
    rd_pointh geometry, normal, dev;

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
    geometry = world_to_clip * geometry; // Run through world-

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
            dev[3] = clipped_list[index].coord[3];
            dev = clip_to_device * dev;

            // Put the X and Y back in as they're the only ones changed meaningfully
            clipped_list[index].coord[0] = dev[0];
            clipped_list[index].coord[1] = dev[1];

            // Perform the perspective divide
            clipped_list[index].coord[0] /= clipped_list[index].coord[3];
            clipped_list[index].coord[1] /= clipped_list[index].coord[3];
            clipped_list[index].coord[2] /= clipped_list[index].coord[3];
        }

        // Pass the clipped geometry into the scan conversion function
        draw_poly(num_vertex, clipped_list);
    }

    // Reset our vertex list for the next polygon
    num_vertex = 0;
    return 0;
}

///
int REDirect::clip_poly(int num_vertex, rd_pointa *vertex_list, rd_pointa *clipped_list)
{
    // Move vertex_list into clipped_list
    // TODO: actually implement this function
    for (int index = 0; index < num_vertex; index++)
        clipped_list[index] = vertex_list[index];

    return num_vertex;
}

///
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

///
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
        if (points[v1].coord[1] != points[v2].coord[1])
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

///
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
    edge->yLast = ceil(upper.coord[1]) - 1;
    insert_edge(&edge_table[(int)ceilf(lower.coord[1])], edge);
}

///
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

///
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

///
void REDirect::update_aet(int scanline, rd_edge *aet)
{
    // Keep track of our active and next active edges
    rd_edge* active = aet, *next = aet->next;

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

///
void REDirect::delete_next_edge(rd_edge *edge)
{
    rd_edge* deletee = edge->next;
    edge->next = deletee->next;
    delete deletee;
}

///
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

///
void REDirect::fill_between_edges(int scanline, rd_edge *aet)
{
    rd_edge* q = aet, *p = aet->next;
    while (q)
    {
        q = p;
        if (q) p = q->next;
    }

    //
    rd_edge* current, *next;

    current = aet->next;
    while (current)
    {
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
                //float color[3] = { value.coord[ATTR_R], value.coord[ATTR_G], value.coord[ATTR_B] };
                rd_pointc point(value.coord[0], scanline, value.coord[2]);
                plot_pixel(point);

                // Increment the values
                value = value + increment;
            }

            // Move to the next edge pair
            current = next->next;
        }
    }
}

/// Renders a simple circle in 3D space, using the line rendering pipeline to
/// draw a continuous set of line segments around the radius of the circle.
/// @param radius A float that represents the radius of the circle.
/// @param z The z-coordinate of the circle.
void REDirect::render_circle(float radius, float z)
{
    // Create a float for our angle
    float angle = 0; // In radians

    // Pass our point into the line pipeline but only move as this is an initial point
    render_line(rd_pointh(radius, angle, z), false);
    for (int index = 1; index <= NUM_SEGMENTS; index++)
    {
        angle = index * 2 * M_PI/NUM_SEGMENTS;
        rd_pointh point(radius * cosf(angle), radius * sinf(angle), z);
        render_line(point, true); // Draw every point in the loop
    }
}
