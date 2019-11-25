#ifndef DATABASE_H
#define DATABASE_H

#include "color_fl.h"

#include <ai.h>

#include <QtCore/QObject>

class PixelBuffer : public QObject
{
    Q_OBJECT

public:
    static PixelBuffer& GetInstance();

    Color_fl* GetPixels();
    void BuildPixels(int size);
    void EmitPixelsReady();

signals:
    void PixelsReady();

private:
    PixelBuffer();
    std::vector<Color_fl> m_pixels;
};

#endif // DATABASE_H
