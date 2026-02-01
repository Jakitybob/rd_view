#include "rd_direct.h"
#include "rd_error.h"

int REDirect::rd_display(const string &name, const string &type, const string &mode)
{
    return RD_OK;
}

int REDirect::rd_format(int xresolution, int yresolution)
{
    return RD_OK;
}

int REDirect::rd_world_begin()
{
    return RD_OK;
}

int REDirect::rd_world_end()
{
    return RD_OK;
}

int REDirect::rd_frame_begin(int frame_no)
{
    return RD_OK;
}

int REDirect::rd_frame_end()
{
    return RD_OK;
}

int REDirect::rd_circle(const float center[3], float radius)
{
    return RD_OK;
}

int REDirect::rd_line(const float start[3], const float end[3])
{
    return RD_OK;
}

int REDirect::rd_point(const float p[3])
{
    return RD_OK;
}

int REDirect::rd_background(const float color[])
{
    return RD_OK;
}

int REDirect::rd_color(const float color[])
{
    return RD_OK;
}

int REDirect::rd_fill(const float seed_point[3])
{
    return RD_OK;
}
