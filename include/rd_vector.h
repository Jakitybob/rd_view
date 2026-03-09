/********************************************************************
CSCI 490J - Assignment 2 - Spring 2026
Programmer: Jacob Yates
Date Due: 03/16/2026

Purpose: This class implements a mathematical vector as well as a
        vector addition, subtractions, dot product, and cross
        multiplication. It also has a utility to quickly find the
        magnitude.
*********************************************************************/

#ifndef RD_VIEW_RD_VECTOR_H
#define RD_VIEW_RD_VECTOR_H

class rd_vector
{
private:
    float x, y, z;

public:
    // Constructors
    rd_vector();
    rd_vector(float x, float y, float z);

    // Operator overloads
    rd_vector operator+ (rd_vector v);
    rd_vector operator- (rd_vector v);
    float operator^ (rd_vector v);
    rd_vector operator* (rd_vector v);
    rd_vector& operator=(const rd_vector& v);

    // Functions
    float magnitude();

    // Getters / Setters
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    void SetX(float x) { this->x = x; }
    void SetY(float y) { this->y = y; }
    void SetZ(float z) { this->z = z; }
};


#endif //RD_VIEW_RD_VECTOR_H