#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextStream>
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

    void on_horizontalSlider_sliderReleased();

    void on_radioButtonFull_clicked(bool checked);

    void on_radioButtonLeftR_clicked(bool checked);

    void on_radioButtonRightR_clicked(bool checked);

    void on_radioButtoLeftB_clicked(bool checked);

    void on_radioButtonRightB_clicked(bool checked);

    void on_horizontalSlider_sliderPressed();

    void on_radioButtonSingle_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    void serializeAndWrite();
    bool sliderPressed = false;
};

#endif // MAINWINDOW_H
