/********************************************************************
CSCI 490J - Assignment 4 - Spring 2026
Programmer: Jacob Yates
Last Updated: 4/18/2026

Purpose: This header implements a simple struct that represents an
        edge to be used during scan conversion for creation of
        an edge table.
*********************************************************************/

#ifndef RD_VIEW_RD_EDGE_H
#define RD_VIEW_RD_EDGE_H
#include "rd_pointa.h"

struct rd_edge
{
    int yLast; // Final scan line of the edge
    rd_pointa point; // The values of the edge on this scanline
    rd_pointa increment; // The incremental changes for the values from scanline to scanline
    rd_edge* next = nullptr; // A pointer
};

#endif //RD_VIEW_RD_EDGE_H
