#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextStream>
#include <QCloseEvent>
#include <QAbstractButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_horizontalSlider_valueChanged(int value);
    void buttonClicked();
    void buttonClicked(QAbstractButton *button);

    void on_horizontalSlider_sliderReleased();

    void on_radioButtonFull_clicked(bool checked);

    void on_radioButtonLeftR_clicked(bool checked);

    void on_radioButtonRightR_clicked(bool checked);

    void on_radioButtoLeftB_clicked(bool checked);

    void on_radioButtonRightB_clicked(bool checked);

    void on_horizontalSlider_sliderPressed();

    void on_radioButtonSingle_clicked(bool checked);

    void on_rightSlider_sliderPressed();

    void on_rightSlider_sliderReleased();

    void on_leftSlider_sliderPressed();

    void on_leftSlider_sliderReleased();

    void on_leftSlider_valueChanged(int value);

    void on_rightSlider_valueChanged(int value);

    void on_btnStart_clicked();

    void on_maxBitRate_returnPressed();

private:
    Ui::MainWindow *ui;
    void serializeAndWrite();
    void changeMode();

    bool sliderPressed = false;
    void closeEvent(QCloseEvent *bar);
    QString input = "BASKETBALL";
};

#endif // MAINWINDOW_H
