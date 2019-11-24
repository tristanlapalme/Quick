#ifndef COLOR_FL_H
#define COLOR_FL_H

struct Color_fl
{
    Color_fl(float vr = 0.0f, float vg = 0.0f, float vb = 0.0f, float va = 0.0f): r(vr), g(vg), b(vb), a(va) {}

    /*! Storage for the floating point color information. */
    float r,g,b,a;

    /*! \remarks Returns the address of the floating point values. */
    operator float*() { return &r; }
    /*! \remarks Returns the address of the floating point values. */
    operator const float*() const { return &r; }
};

#endif // COLOR_FL_H
