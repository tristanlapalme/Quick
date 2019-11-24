#ifndef EDITCLEARWIDGET_H
#define EDITCLEARWIDGET_H

#include <QWidget>

namespace Ui {
class EditClearWidget;
}

class EditClearWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditClearWidget(QWidget *parent = nullptr);
    ~EditClearWidget();

    QString text() const;
    void setText(const QString& text);
    void setPlaceholderText(const QString& text) const;

signals:
    void textEdited();
    void textCleared();

private:
    Ui::EditClearWidget *ui;
};

#endif // EDITCLEARWIDGET_H
