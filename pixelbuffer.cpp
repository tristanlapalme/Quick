#include "pixelbuffer.h"

PixelBuffer& PixelBuffer::GetInstance()
{
    static PixelBuffer instance;
    return instance;
}

PixelBuffer::PixelBuffer()
{

}

Color_fl* PixelBuffer::GetPixels()
{
    return m_pixels.data();
}

void PixelBuffer::BuildPixels(int size)
{
    m_pixels.resize(size);
}

void PixelBuffer::EmitPixelsReady()
{
    emit PixelsReady();
}
