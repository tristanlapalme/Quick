#include "editclearwidget.h"
#include "ui_editclearwidget.h"

EditClearWidget::EditClearWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditClearWidget)
{
    ui->setupUi(this);

    //ui->pushButton->setFlat(true);

    connect(ui->lineEdit, &QLineEdit::textEdited, this, [=] { emit textEdited();});
    connect(ui->pushButton, &QPushButton::clicked, this, [=] { ui->lineEdit->setText(""); emit textEdited();});
}

EditClearWidget::~EditClearWidget()
{
    delete ui;
}

QString EditClearWidget::text() const
{
    return ui->lineEdit->text();
}

void EditClearWidget::setText(const QString &text)
{
    ui->lineEdit->setText(text);
}

void EditClearWidget::setPlaceholderText(const QString& text) const
{
    ui->lineEdit->setPlaceholderText(text);
}
