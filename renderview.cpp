#include "renderview.h"

#include "pixelbuffer.h"

#include <QtGui/QPainter>
#include <QtDebug>

RenderView::RenderView(QWidget* parent)
    : QWidget(parent)
{
    //int w = width();
    //int h = height();

    int w = 400;
    int h = 400;

    int s = w*h;
    m_pixels = new QRgb[s];
    m_image = QImage(reinterpret_cast<uchar*>(m_pixels), w, h, QImage::Format_ARGB32);

}

RenderView::~RenderView()
{
    delete[] m_pixels;
    m_pixels = nullptr;
}

void RenderView::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.drawImage(0, 0, m_image);
}

void RenderView::resizeEvent(QResizeEvent* event)
{
    /*
    int w = width();
    int h = height();
    int s = w*h;

    if(m_pixels != nullptr)
    {
        delete[] m_pixels;
    }

    m_pixels = new QRgb[s];
    m_image = QImage(reinterpret_cast<uchar*>(m_pixels), w, h, QImage::Format_ARGB32);
    */
    QWidget::resizeEvent(event);
}

void RenderView::ConsumePixels()
{
    ShowIt();
}

void RenderView::ShowIt()
{
    int size = 400*400;
    Color_fl* colors = PixelBuffer::GetInstance().GetPixels();

    for(int i=0; i<size; i++)
    {
        Color_fl* c = colors+i;
        m_pixels[i]= qRgb(c->r*255.f, c->g*255.f ,c->b*255.f);
    }

    this->repaint();
}
