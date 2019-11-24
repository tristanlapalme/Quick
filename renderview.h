#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QWidget>

class RenderView : public QWidget
{
    Q_OBJECT

public:
    RenderView(QWidget* parent = nullptr);
    virtual ~RenderView();

    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    void ShowIt();

public slots:
    void ConsumePixels();

private:
    QImage m_image;
    QRgb* m_pixels {nullptr};
};

#endif // RENDERVIEW_H
