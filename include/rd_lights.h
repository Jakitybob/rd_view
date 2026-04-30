/********************************************************************
CSCI 490J - Assignment 5 - Spring 2026
Programmer: Jacob Yates
Last Updated: 04/30/2026

Purpose: This header implements the three structs for the lights
        required for assignment 5: the ambient light, far lights,
        and point lights.
*********************************************************************/

#ifndef RD_VIEW_RD_LIGHTS_H
#define RD_VIEW_RD_LIGHTS_H

#include "rd_vector.h"
#include "rd_pointc.h"

// The ambient light, of which only one should exist per scene,
// is defined only by light intensity values.
struct rd_ambient_light
{
    float redIntensity, greenIntensity, blueIntensity;
};

// Far lights are defined not only by their color intensity but
// by the direction they are pointing in as well.
struct rd_far_light
{
    float redIntensity, greenIntensity, blueIntensity;
    rd_vector direction;
};

// Point lights are defined by color intensity and their position.
// They emit light in all directions equally from that point, kind
// of like a lamp or a torch.
struct rd_point_light
{
    float redIntensity, greenIntensity, blueIntensity;
    rd_pointc positon;
};

#endif //RD_VIEW_RD_LIGHTS_H
