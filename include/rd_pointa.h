/********************************************************************
CSCI 490J - Assignment 4 - Spring 2026
Programmer: Jacob Yates
Last Updated: 4/18/2026

Purpose: This header implements a simple attributed point
        structure that contains lots of important data for our
        polygon and lighting pipelines.
*********************************************************************/

#ifndef RD_VIEW_RD_POINTA_H
#define RD_VIEW_RD_POINTA_H

#define ATTR_CONSTANT 4
#define ATTR_R 5
#define ATTR_G 6
#define ATTR_B 7
#define ATTR_NX 8
#define ATTR_NY 9
#define ATTR_NZ 10
#define ATTR_S        11
#define ATTR_T        12
#define ATTR_WORLD_X  13
#define ATTR_WORLD_Y  14
#define ATTR_WORLD_Z  15
#define ATTR_SIZE     16

class rd_pointa
{
public:
    // Size 16 array of floats.
    float coord[ATTR_SIZE];

    // Constructors
    rd_pointa();
    rd_pointa(float x, float y, float z, float w);
    rd_pointa(const rd_pointa& point);

    // Operator Overloads
    rd_pointa operator+ (rd_pointa point);
    rd_pointa operator- (rd_pointa point);
    rd_pointa operator/ (float divisor);
    friend rd_pointa operator* (float factor, const rd_pointa& point); // Non-member overload for left-hand float mult

    // Member / utility functions for easier access
    void set_normal(float nx, float ny, float nz);
};

#endif //RD_VIEW_RD_POINTA_H
