/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a simple 3D point with a number
        of operator overloads to allow easier use of points in the
        RD View application.
*********************************************************************/

#ifndef RD_VIEW_RD_POINT_H
#define RD_VIEW_RD_POINT_H

class rd_pointc
{
private:
    float x, y, z;

public:
    // Constructors
    rd_pointc();
    rd_pointc(float x, float y, float z);
    rd_pointc(const float* point); // Using float array
    rd_pointc(class rd_pointh point); // Homogenous to cartesian

    // Operator overloads
    class rd_vector operator- (rd_pointc point);
    rd_pointc operator* (float scalar);
    rd_pointc& operator= (rd_pointc point);

    // Getters / Setters
    float get_x() const { return x; }
    float get_y() const { return y; }
    float get_z() const { return z; }
    void set_x(float x) { this->x = x; }
    void set_y(float y) { this->y = y; }
    void set_z(float z) { this->z = z; }
};

#endif //RD_VIEW_RD_POINT_H