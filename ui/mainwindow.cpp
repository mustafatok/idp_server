#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serializeAndWrite();

    ui->horizontalLayout_2->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_3->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_4->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_7->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_8->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_9->setAlignment(Qt::AlignTop);


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->labelLeft->setNum(1000 - value);
    ui->labelRight->setNum(value);
    //serializeAndWrite();
}

void MainWindow::serializeAndWrite()
{
    if(ui->stdX->text() == "" || ui->stdY->text() == ""  || ui->sizeX->text() == "")
        return;
    QString str;
    str.append(ui->modeButtonGroup->checkedButton()->toolTip());
    str.append(QString(";").append(ui->labelRight->text()));
    str.append(QString(";").append(ui->sizeButtonGroup->checkedButton()->toolTip()));
    str.append(QString(";").append(ui->sizeX->text()));
    str.append(QString(";").append(ui->stdX->text()));
    str.append(QString(";").append(ui->stdY->text()));
    std::cout << str.toStdString() << std::endl;
}


void MainWindow::buttonClicked()
{
    serializeAndWrite();
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    serializeAndWrite();
}

void MainWindow::on_radioButtonFull_clicked(bool checked)
{
    ui->horizontalSlider->setValue(500);
    ui->horizontalSlider->setEnabled(false);
    serializeAndWrite();
}

void MainWindow::on_radioButtonLeftR_clicked(bool checked)
{
    ui->horizontalSlider->setValue(800);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
}

void MainWindow::on_radioButtonRightR_clicked(bool checked)
{
    ui->horizontalSlider->setValue(200);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
}

void MainWindow::on_radioButtoLeftB_clicked(bool checked)
{
    ui->horizontalSlider->setValue(600);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
}

void MainWindow::on_radioButtonRightB_clicked(bool checked)
{
    ui->horizontalSlider->setValue(400);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
}
