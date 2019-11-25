#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <ai.h>

#include "filter.h"
#include "nodeentrymodel.h"
#include "quickparamwidget.h"
#include "quickscenewidget.h"


#include <QtGui/QStandardItem>
#include <QtCore/QFile>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Arnold Quick Param (quick.exe)");
    this->setWindowFlags(Qt::Window);
    ui->statusBar->hide();
    ui->menuBar->hide();
    ui->mainToolBar->hide();

    ui->paramTab->layout()->addWidget(new QuickParamWidget(this));
    ui->sceneTab->layout()->addWidget(new QuickSceneWidget(this));

    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
