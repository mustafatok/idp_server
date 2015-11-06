#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QSound>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->horizontalLayout_3->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_4->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_7->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_8->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_9->setAlignment(Qt::AlignTop);

    QObject::connect(ui->inputButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
                       this, SLOT(buttonClicked(QAbstractButton*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if(value >= ui->horizontalSlider->maximum() || value <= 0 ){
        if(ui->modeButtonGroup->checkedButton() != ui->radioButtonFull && ui->modeButtonGroup->checkedButton() != ui->radioButtonSingle )
            QSound::play("beep.wav");
    }

    ui->labelLeft->setNum(ui->horizontalSlider->maximum() - value);
    ui->labelRight->setNum(value);
    if(sliderPressed != true) serializeAndWrite();
}

void MainWindow::serializeAndWrite()
{
    if(ui->btnStart->isEnabled()) return;
    if(ui->stdX->text() == "" || ui->stdY->text() == ""  || ui->sizeX->text() == "")
        return;
    QString str;
    str.append(ui->modeButtonGroup->checkedButton()->toolTip());
    str.append(QString(";").append(ui->labelLeft->text()));
    str.append(QString(";").append(ui->labelRight->text()));
    str.append(QString(";").append(ui->sizeButtonGroup->checkedButton()->toolTip()));
    str.append(QString(";").append(ui->sizeX->text()));
    str.append(QString(";").append(ui->stdX->text()));
    str.append(QString(";").append(ui->stdY->text()));
    str.append(QString(";").append(QString::number(ui->leftSlider->value())));
    str.append(QString(";").append(QString::number(ui->rightSlider->value())));
    str.append(QString(";").append(input));

    std::cout << str.toStdString() << std::endl;
}
void MainWindow::changeMode()
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(ui->horizontalSlider->maximum() / 2);
    serializeAndWrite();
    sliderPressed = false;
}


void MainWindow::buttonClicked()
{
    serializeAndWrite();
}

void MainWindow::buttonClicked(QAbstractButton *button)
{
    input = button->text();
    if(!ui->btnStart->isEnabled())
        serializeAndWrite();

    foreach(QAbstractButton *b, ui->inputButtonGroup->buttons()){
          b->setStyleSheet("");
    }
    button->setStyleSheet("background-color: blue; color: white");
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    serializeAndWrite();
    sliderPressed = false;
}

//void MainWindow::on_radioButtonFull_clicked(bool checked)
//{
//    ui->horizontalSlider->setEnabled(false);
//    changeMode();
//}

//void MainWindow::on_radioButtonLeftR_clicked(bool checked)
//{
//    ui->horizontalSlider->setEnabled(true);
//    changeMode();
//}

//void MainWindow::on_radioButtonRightR_clicked(bool checked)
//{
//    ui->horizontalSlider->setEnabled(true);
//    changeMode();
//}

//void MainWindow::on_radioButtoLeftB_clicked(bool checked)
//{
//    ui->horizontalSlider->setEnabled(true);
//    changeMode();
//}

//void MainWindow::on_radioButtonRightB_clicked(bool checked)
//{
//    ui->horizontalSlider->setEnabled(true);
//    changeMode();
//}
void MainWindow::on_radioButtonFull_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(0);
    ui->horizontalSlider->setEnabled(false);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_radioButtonLeftR_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(1000);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_radioButtonRightR_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(500);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_radioButtoLeftB_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(600);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_radioButtonRightB_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(400);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_horizontalSlider_sliderPressed()
{
    sliderPressed = true;
}

void MainWindow::on_radioButtonSingle_clicked(bool checked)
{
    sliderPressed = true;
    ui->horizontalSlider->setValue(0);
    ui->horizontalSlider->setEnabled(true);
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_rightSlider_sliderPressed()
{
    sliderPressed = true;
}

void MainWindow::on_rightSlider_sliderReleased()
{
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_leftSlider_sliderPressed()
{
    sliderPressed = true;
}

void MainWindow::on_leftSlider_sliderReleased()
{
    serializeAndWrite();
    sliderPressed = false;
}

void MainWindow::on_leftSlider_valueChanged(int value)
{
    if(sliderPressed != true) serializeAndWrite();
}

void MainWindow::on_rightSlider_valueChanged(int value)
{
    if(sliderPressed != true) serializeAndWrite();
}

void MainWindow::closeEvent(QCloseEvent *bar)
{
    std::cout << "exit" << std::endl;
    bar->accept();
}

void MainWindow::on_btnStart_clicked()
{

    ui->btnStart->setEnabled(false);
    serializeAndWrite();
}

void MainWindow::on_maxBitRate_returnPressed()
{
    int nMax = ui->maxBitRate->text().toInt();
    int oMax = ui->horizontalSlider->maximum();

    int right = (nMax / oMax) * ui->horizontalSlider->value();
    ui->labelLeft->setNum(nMax - right);
    ui->labelRight->setNum(right);

    sliderPressed = true;
    ui->horizontalSlider->setMaximum(nMax);
    ui->horizontalSlider->setValue(right);
    sliderPressed = false;

}
