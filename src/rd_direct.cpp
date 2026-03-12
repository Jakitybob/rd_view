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
#include "rd_pointc.h"
#include "rd_pointh.h"

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

    // Calculate our graphics pipeline matrices
    calculate_world_to_clip();
    calculate_clip_to_device();

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

///
int REDirect::rd_cone(float height, float radius, float thetamax)
{
    return RD_OK;
}

/// Draws a cube centered about the origin with a length and height of one on each
/// side. Each line of the cube is passed into the line pipeline where it will be
/// transformed and sent through the world -> device pipeline.
int REDirect::rd_cube()
{
    // Draw the bottom face of the square
    rd_line(new float[3] {-1, -1, -1}, new float[3] {-1, -1, 1});
    rd_line(new float[3] {-1, -1, 1}, new float[3] {1, -1, 1});
    rd_line(new float[3] {1, -1, 1}, new float[3] {1, -1, -1});
    rd_line(new float[3] {1, -1, -1}, new float[3] {-1, -1, -1});

    // Draw the top face of the square
    rd_line(new float[3] {1, 1, 1}, new float[3] {1, 1, -1});
    rd_line(new float[3] {1, 1, -1}, new float[3] {-1, 1, -1});
    rd_line(new float[3] {-1, 1, -1}, new float[3] {-1, 1, 1});
    rd_line(new float[3] {-1, 1, 1}, new float[3] {1, 1, 1});

    // Connect the top and the bottom
    rd_line(new float[3] {1, 1, 1}, new float[3] {1, -1, 1});
    rd_line(new float[3] {-1, 1, 1}, new float[3] {-1, -1, 1});
    rd_line(new float[3] {-1, 1, -1}, new float[3] {-1, -1, -1});
    rd_line(new float[3] {1, 1, -1}, new float[3] {1, -1, -1});

    return RD_OK;
}

/// Draws a cylinder of the given radius with height zmax - zmin.
/// @param radius The radius of the cylinder.
/// @param zmin The z-coordinate of the bottom of the cylinder.
/// @param zmax The z-coordinate of the top of the cylinder.
/// @param thetamax UNUSED.
int REDirect::rd_cylinder(float radius, float zmin, float zmax, float thetamax)
{
    // Manually draw the top circle and draw extra line segments down to the bottom circle
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

///
int REDirect::rd_disk(float height, float radius, float theta)
{
    return RD_OK;
}

/// Draws three circles in 3D space, with one on the XY plane, one on the
/// YZ plane, and a third on the ZX plane. A simple representation of a sphere
/// that will be redone shortly for assignment 3.
/// @param radius The radius of the sphere.
/// @param zmin UNUSED.
/// @param zmax UNUSED.
/// @param thetamax UNUSED.
int REDirect::rd_sphere(float radius, float zmin, float zmax, float thetamax)
{
    // Store our current transform on the stack
    rd_xform_push();

    // Render a circle on the XY plane
    render_circle(radius, 0);

    // Rotate so that the circle draws about the YZ plane
    rd_rotate_yz(90);
    render_circle(radius, 0);

    // Rotate so that the circle draws about the ZX plane
    rd_rotate_zx(90);
    render_circle(radius, 0);

    // Pop the stack so the transform returns to its prior state
    rd_xform_pop();

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

/// A function that calculates the world to clip transformation matrix and
/// stores it our into our global world to clip matrix variable.
void REDirect::calculate_world_to_clip()
{
    // Create our camera vectors to create the view matrix
    rd_vector world_up(camera_up);
    rd_vector forward = (camera_at - camera_eye).normalized();
    rd_vector right = (world_up * forward).normalized();
    rd_vector up = (right * forward).normalized();

    // Create the view matrix using the forward, right, and up vectors from the camera
    rd_xform view_matrix = {
        right.GetX(), right.GetY(), right.GetZ(), -(right ^ camera_eye),
        up.GetX(), up.GetY(), up.GetZ(), -(up ^ camera_eye),
        forward.GetX(), forward.GetY(), forward.GetZ(), -(forward ^ camera_eye),
        0, 0, 0, 1
    };

    // Create our perspective transform matrix
    float fov_scale = tanf((camera_fov / 2) * (std::numbers::pi / 180));
    float aspect_ratio = display_xSize / display_ySize;
    rd_xform perspective_matrix = {
        1/(aspect_ratio * fov_scale), 0, 0, 0,
        0, 1/fov_scale, 0, 0,
        0, 0, (far_clip + near_clip)/(near_clip - far_clip), (2 * far_clip * near_clip)/(near_clip - far_clip),
        0, 0, -1, 0
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
        (float)(display_ySize / 2), 0, 0, (float)(display_xSize / 2),
        0, (float)(display_ySize / 2), 0, (float)(display_ySize / 2),
        0, 0, 0.5, 0.5,
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

    // Convert our point back to cartesian coordinates and draw it
    rd_pointc cartesian_point = rd_pointc(point);

    // Write our pixel to the screen after having run it through the pipeline
    rd_write_pixel((int)(cartesian_point.get_x()), (int)(cartesian_point.get_y()), new float[3] { drawRed, drawGreen, drawBlue });
}

/// Creates boundary coordinates from Brinn's clipping algorithm and if any of the
/// boundary coordinates are negative, returns true to clip the point.
/// @param point A homogeneous point to check whether it is in bounds.
/// @returns True if the point should be clipped and false if not.
bool REDirect::check_point_clip(class rd_pointh point)
{
    // Create a simple boundary coordinate array
    float boundary_coordinates[6] = {
        point.get_x() + point.get_w(), point.get_w() - point.get_x(),
        point.get_y() + point.get_w(), point.get_w() - point.get_y(),
        point.get_z() + point.get_w(), point.get_w() - point.get_z()
    };

    // Traverse the boundary coordinates and return true to clip the point if any coord is negative
    for (int index = 0; index < 6; index++)
        if (boundary_coordinates[index] < 0) return true;

    // Return false, we should not clip this point, if we made it past the for loop
    return false;
}

/// Runs the specified point through world->device transformation pipeline
/// and, if the flag is on, draws a line using the DDA algorithm from the
/// last vertex passed in to the current vertex.
/// @param point The homogeneous point to plot as a vertex.
/// @param should_draw Whether a line should be drawn between this vertex and the last.
void REDirect::render_line(rd_pointh point, bool should_draw)
{
    // Run our point through our current transformations
    point = current_transform * point;

    // Run our point through the world->clip transformation
    point = world_to_clip * point;

    // If the point should be drawn, render a line between this point and the last vertex
    if (should_draw)
    {
        // Convert the last vertex and this vertex to device coordinates
        rd_pointc start_vertex = rd_pointc(clip_to_device * last_vertex);
        rd_pointc end_vertex = rd_pointc(clip_to_device * point);

        // Calculate dX and dY, the difference in endpoints
        int dX = abs((int)start_vertex.get_x() - (int)end_vertex.get_x());
        int dY = abs((int)start_vertex.get_y() - (int)end_vertex.get_y());

        // Store our number of steps based on the max between dX and dY
        const int NSTEPS = std::max(dX, dY);

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

            // Plot our pixel on the screen at the x and y
            rd_write_pixel((int)x, int(y), new float[3] { drawRed, drawGreen, drawBlue });
        }
    }

    // Update our last vertex variable either way
    last_vertex = point;
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
