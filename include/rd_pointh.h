/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a simple 3D homogenous point with a number
        of operator overloads to allow easier use of points in the
        RD View application.
*********************************************************************/

#ifndef RD_VIEW_RD_POINTH_H
#define RD_VIEW_RD_POINTH_H


class rd_pointh
{
private:
    float x, y, z, w;

public:
    // Constructors
    rd_pointh();
    rd_pointh(float x, float y, float z);
    rd_pointh(const float* point);
    rd_pointh(class rd_point point);
    rd_pointh(float x, float y, float z, float w);

    // Getters / setters
    float get_x() const { return x; }
    float get_y() const { return y; }
    float get_z() const { return z; }
    float get_w() const { return w; }
    void set_x(float x) { this->x = x; }
    void set_y(float y) { this->y = y; }
    void set_z(float z) { this->z = z; }
    void set_w(float w) { this->w = w; }
};


#endif //RD_VIEW_RD_POINTH_H