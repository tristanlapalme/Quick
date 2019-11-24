#ifndef ARNOLDRIVER_H
#define ARNOLDRIVER_H

#include "color_fl.h"

#include <ai.h>
#include <unordered_map>

class DriverData
{
public:
    int m_xres = 0, m_yres = 0;

    DriverData() {}
    ~DriverData() {}

    Color_fl* m_pixels{nullptr};

    void Initialize();
    void PutPixel(const std::string& aovName, const AtRGB &in_rgb, const int in_x, int in_y);
    void PutPixel(const std::string& aovName, const AtRGBA &in_rgba, const int in_x, int in_y);
    void PutPixel(const std::string& aovName, const float in_f, const int in_x, int in_y);
    void PutPixel(const std::string& aovName, const int in_i, const int in_x, int in_y);

    void DrawCorners(const std::string& aovName, const int in_min_x, const int in_min_y, const int in_max_x, const int in_max_y, const int in_tid);

private:
    void PutPixelsRow(const std::string& aovName, const int in_x, const int in_y, const Color_fl &in_color, const int in_nb_pixels);
    void PutPixelsColumn(const std::string& aovName, const int in_x, const int in_y, const Color_fl &in_color, const int in_nb_pixels);
};

void InitializeArnoldDriver();

#endif // ARNOLDRIVER_H
