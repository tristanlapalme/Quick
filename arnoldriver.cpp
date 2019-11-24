#include "arnoldriver.h"
#include "database.h"

#include <QtDebug>

AI_DRIVER_NODE_EXPORT_METHODS(driver_mtd);

void DriverData::Initialize()
{
    AtNode* options = AiUniverseGetOptions();
    m_xres = AiNodeGetInt(options, "xres");
    m_yres = AiNodeGetInt(options, "yres");

    m_pixels = Database::GetInstance().GetPixels();
}

void DriverData::PutPixel(const std::string& aovName, const AtRGB &in_rgb, const int in_x, const int in_y)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];
    pixel->r = in_rgb.r;
    pixel->g = in_rgb.g;
    pixel->b = in_rgb.b;
    pixel->a = 1.0f;
}

void DriverData::PutPixel(const std::string& aovName, const AtRGBA &in_rgba, const int in_x, const int in_y)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];
    pixel->r = in_rgba.r;
    pixel->g = in_rgba.g;
    pixel->b = in_rgba.b;
    pixel->a = in_rgba.a;
}

void DriverData::PutPixel(const std::string& aovName, const float in_f, const int in_x, const int in_y)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];
    pixel->r = pixel->g = pixel->b = in_f;
    pixel->a = 1.0f;
}

// Copied from the Arnold Render View, original code by Brecht.
// The only difference is that we output black for an input of 0, and alpha = input != 0
//
void DriverData::PutPixel(const std::string& aovName, const int in_i, const int in_x, const int in_y)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];

    if (in_i == 0)
    {
        pixel->r = pixel->g = pixel->b = pixel->a = 0.0f;
        return;
    }

    float h = (in_i >> 16) / 65536.0f; // [0,1)
    float v = ((in_i >> 8) & 0xFF) / 255.0f;
    v = AiLerp(v, 1.00f, 0.5f);
    float s = ((in_i) & 0xFF) / 255.0f;
    s = AiLerp(s, 0.95f, 0.5f);
    h = 6 * h;
    int hi = (int)h;
    float f = h - hi;
    float p = v * (1 - s);
    float q = v * (1 - s * f);
    float t = v * (1 - s * (1 - f));

    if      (hi == 0) { pixel->r = v; pixel->g = t; pixel->b = p; }
    else if (hi == 1) { pixel->r = q; pixel->g = v; pixel->b = p; }
    else if (hi == 2) { pixel->r = p; pixel->g = v; pixel->b = t; }
    else if (hi == 3) { pixel->r = p; pixel->g = q; pixel->b = v; }
    else if (hi == 4) { pixel->r = t; pixel->g = p; pixel->b = v; }
    else              { pixel->r = v; pixel->g = p; pixel->b = q; }

    pixel->a = 1.0f;
}

#define CORNER_LENGTH 6  // length in pixels of the corners
#define NB_CORNER_COLORS 8

static Color_fl CornerColors[NB_CORNER_COLORS] =
{
    { 1.0f, 1.0f, 1.0f, 1.0f }, // white
    { 1.0f, 0.0f, 0.0f, 1.0f }, // red
    { 0.0f, 1.0f, 0.0f, 1.0f }, // green
    { 0.0f, 0.0f, 1.0f, 1.0f }, // blu
    { 1.0f, 1.0f, 0.0f, 1.0f }, // yellow
    { 1.0f, 0.0f, 1.0f, 1.0f }, // magenta
    { 0.0f, 1.0f, 1.0f, 1.0f }, // cyan
    { 1.0f, 0.5f, 0.0f, 1.0f }  // orange
};

void DriverData::DrawCorners(const std::string& aovName, const int in_min_x, const int in_min_y, const int in_max_x, const int in_max_y, const int in_tid)
{
    Color_fl& corner_color = CornerColors[in_tid % NB_CORNER_COLORS];

    int bucket_size_x = in_max_x - in_min_x + 1;
    int bucket_size_y = in_max_y - in_min_y + 1;
    // buckets at the edges of the buffer can have small width/height
    int nb_pixels_x = AiMin(CORNER_LENGTH, bucket_size_x);
    int nb_pixels_y = AiMin(CORNER_LENGTH, bucket_size_y);

    // rows:
    // bottom left, top left
    PutPixelsRow(aovName, in_min_x, in_min_y, corner_color, nb_pixels_x);
    PutPixelsRow(aovName, in_min_x, in_max_y, corner_color, nb_pixels_x);
    // bottom right, top right
    if (bucket_size_x > nb_pixels_x)
    {
        PutPixelsRow(aovName, in_max_x - nb_pixels_x + 1, in_min_y, corner_color, nb_pixels_x);
        PutPixelsRow(aovName, in_max_x - nb_pixels_x + 1, in_max_y, corner_color, nb_pixels_x);
    }

    // columns:
    // bottom left, bottom right
    PutPixelsColumn(aovName, in_min_x, in_min_y, corner_color, nb_pixels_y);
    PutPixelsColumn(aovName, in_max_x, in_min_y, corner_color, nb_pixels_y);
    // top left, top right
    if (bucket_size_y > nb_pixels_y)
    {
        PutPixelsColumn(aovName, in_min_x, in_max_y - nb_pixels_y + 1, corner_color, nb_pixels_y);
        PutPixelsColumn(aovName, in_max_x, in_max_y - nb_pixels_y + 1, corner_color, nb_pixels_y);
    }
}

void DriverData::PutPixelsRow(const std::string& aovName, const int in_x, const int in_y, const Color_fl &in_color, const int in_nb_pixels)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];
    int nb_pixels = in_x + in_nb_pixels > m_xres ? m_xres - in_x : in_nb_pixels;

    for (int i = 0; i < nb_pixels; i++, pixel++)
        *pixel = in_color;
}

void DriverData::PutPixelsColumn(const std::string& aovName, const int in_x, const int in_y, const Color_fl &in_color, const int in_nb_pixels)
{
    if (in_x < 0 || in_y < 0 || in_x > m_xres - 1 || in_y > m_yres - 1)
        return;

    Color_fl *pixel = &m_pixels[m_xres * in_y + in_x];
    int nb_pixels = in_y + in_nb_pixels > m_yres ? m_yres - in_y : in_nb_pixels;

    for (int i = 0; i < nb_pixels; i++, pixel+= m_xres)
        *pixel = in_color;
}

void InitializeArnoldDriver()
{
    qDebug() << "InitializeArnoldDriver";
    AiNodeEntryInstall(AI_NODE_DRIVER, AI_TYPE_NONE, "max_driver", nullptr, (AtNodeMethods*)driver_mtd, AI_VERSION);
}

node_parameters {}

node_initialize
{
    DriverData* driver_data = new DriverData;
    AiNodeSetLocalData(node, driver_data);
    AiDriverInitialize(node, true);
}

node_update
{
    DriverData *driver_data = (DriverData*)AiNodeGetLocalData(node);
    driver_data->Initialize();
}

driver_supports_pixel_type
{
    return pixel_type == AI_TYPE_RGB || pixel_type == AI_TYPE_RGBA ||
           pixel_type == AI_TYPE_VECTOR || pixel_type == AI_TYPE_FLOAT ||
           pixel_type == AI_TYPE_INT || pixel_type == AI_TYPE_UINT;
}

driver_extension { return NULL; }

driver_open
{

}

driver_prepare_bucket
{
}

driver_process_bucket
{
    DriverData *driver_data = (DriverData*)AiNodeGetLocalData(node);

    int         pixel_type;
    const void* bucket_data;
    const char* name;

    // iterates for every AOV - and capture pixel in proper bitmap
    while (AiOutputIteratorGetNext(iterator, &name, &pixel_type, &bucket_data))
    {
        //CStr name_utf8 = TSTR::FromUTF8(name);
        //std::string aov_name(name_utf8.data(), name_utf8.length());
        std::string aov_name("");
        int min_x = bucket_xo;
        int max_x = bucket_xo + bucket_size_x - 1;
        int min_y = bucket_yo;
        int max_y = bucket_yo + bucket_size_y - 1;

        switch (pixel_type)
        {
            case AI_TYPE_RGB:
            case AI_TYPE_VECTOR:
            {
                AtRGB* bucket_rgb = (AtRGB*)bucket_data;
                for (int j = min_y; j <= max_y; ++j)
                {
                    int row_start = (j - bucket_yo) * bucket_size_x - bucket_xo;
                    for (int i = min_x; i <= max_x; ++i)
                        driver_data->PutPixel(aov_name, bucket_rgb[row_start + i], i, j);
                }
                break;
            }

            case AI_TYPE_RGBA:
            {
                AtRGBA* bucket_rgba = (AtRGBA*)bucket_data;
                for (int j = min_y; j <= max_y; ++j)
                {
                    int row_start = (j - bucket_yo) * bucket_size_x - bucket_xo;
                    for (int i = min_x; i <= max_x; ++i)
                        driver_data->PutPixel(aov_name, bucket_rgba[row_start + i], i, j);
                }
                break;
            }

            case AI_TYPE_FLOAT:
            {
                float* bucket_f = (float*)bucket_data;
                for (int j = min_y; j <= max_y; ++j)
                {
                    int row_start = (j - bucket_yo) * bucket_size_x - bucket_xo;
                    for (int i = min_x; i <= max_x; ++i)
                        driver_data->PutPixel(aov_name, bucket_f[row_start + i], i, j);
                }
                break;
            }
            case AI_TYPE_INT:
            case AI_TYPE_UINT:
            {
                int* bucket_i = (int*)bucket_data;
                for (int j = min_y; j <= max_y; ++j)
                {
                    int row_start = (j - bucket_yo) * bucket_size_x - bucket_xo;
                    for (int i = min_x; i <= max_x; ++i)
                        driver_data->PutPixel(aov_name, bucket_i[row_start + i], i, j);
                }
                break;
            }
        }
    }

    Database::GetInstance().EmitPixelsReady();
}

driver_close {}

driver_needs_bucket { return true; }

driver_write_bucket {}

node_finish
{
    DriverData *driver_data = (DriverData*)AiNodeGetLocalData(node);
    delete driver_data;
}

