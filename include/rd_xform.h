/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a 4x4 matrix used in RDView for transform
        matrices like translation, rotation, and scaling.
*********************************************************************/

#ifndef RD_VIEW_RD_XFORM_H
#define RD_VIEW_RD_XFORM_H

class rd_xform
{
private:
    float matrix[4][4];

public:
    // Constructors
    rd_xform();

    // Operator Overloads
    rd_xform operator* (rd_xform m2);
    rd_xform& operator= (rd_xform m2);

    // Member functions
    void set_translation(float tx, float ty, float tz);
    void set_scale(float sx, float sy, float sz);
    void set_xy_rotation(float angle); // top left
    void set_yz_rotation(float angle); // middle
    void set_zx_rotation(float angle); // spread out
    void set_identity();

    // Debug / Testing
    void print_matrix();
};


#endif //RD_VIEW_RD_XFORM_H