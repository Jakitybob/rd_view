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
    rd_vector(class rd_pointc point);

    // Operator overloads
    rd_vector operator+ (rd_vector v); // Vector + vector
    rd_vector operator- (rd_vector v); // Vector - vector
    float operator^ (rd_vector v); // Vector ^ vector (dot product)
    float operator^ (class rd_pointc); // Vector ^ cartesian point (dot product)
    rd_vector operator* (rd_vector v); // Vector * vector (cross product)
    rd_vector operator* (float scalar); // Vector * scalar
    friend rd_vector operator* (float factor, const rd_vector& vector);
    rd_vector& operator=(const rd_vector& v); // Vector = vector
    float& operator[](int index);

    // Functions
    float magnitude();
    rd_vector normalized();
    rd_vector flipped_vector();
    static rd_vector reflect(rd_vector light, rd_vector normal);

    // Getters / Setters
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    void SetX(float x) { this->x = x; }
    void SetY(float y) { this->y = y; }
    void SetZ(float z) { this->z = z; }

    // Debug
    void print_vector();
};


#endif //RD_VIEW_RD_VECTOR_H