#include "pnm_display.h"
#include "rd_error.h"

int pnm_init_display()
{
    return RD_OK;
}

int pnm_end_display()
{
    return RD_OK;
}

int pnm_init_frame(int)
{
    return RD_OK;
}

int pnm_end_frame()
{
    return RD_OK;
}

int pnm_write_pixel(int x, int y, const float rgb[])
{
    return RD_OK;
}

int pnm_read_pixel(int x, int y, float rgb[])
{
    return RD_OK;
}

int pnm_set_background(const float rgb[])
{
    return RD_OK;
}

int pnm_clear()
{
    return RD_OK;
}
