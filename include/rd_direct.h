#ifndef RD_ENGINE_DIRECT_H
#define RD_ENGINE_DIRECT_H

#include "rd_enginebase.h"

#include <string>
using std::string;

// This is a rendering engine that renders directly to the image buffer as
// primitives come in.  A depth buffer is obviously needed.  Transparency is
// not properly handled.

class REDirect: public RenderEngine
{
private:
    int frameNumber = 0; // Current frame

public:
    int rd_display(const string &name, const string &type, const string &mode) override;
    int rd_format(int xresolution, int yresolution) override;
    int rd_world_begin() override;
    int rd_world_end() override;
    int rd_frame_begin(int frame_no) override;
    int rd_frame_end() override;
    int rd_circle(const float center[3], float radius) override;
    int rd_line(const float start[3], const float end[3]) override;
    int rd_point(const float p[3]) override;
    int rd_background(const float color[]) override;
    int rd_color(const float color[]) override;
    int rd_fill(const float seed_point[3]) override;
};

#endif /* RD_ENGINE_DIRECT_H */
