#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <vector>
#include <cstdio>
#include <iostream>
#include "f32vec4.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MyWidget; }
QT_END_NAMESPACE

class MyWidget : public QWidget
{
    Q_OBJECT

public:
    MyWidget(QWidget *parent = nullptr);
    void iterration(int iter);
    vector<vector<float>> circle_generate(float center_x,float center_y);
    vector<vector<float>> normalize(vector<vector<float>> C);
    ~MyWidget();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void on_pushButton_clicked();

    void on_Alpha_SpinBox_valueChanged(float arg1);

    void on_Beta_SpinBox_valueChanged(float arg1);

    void on_Gamma_SpinBox_valueChanged(float arg1);

    void on_K_SpinBox_valueChanged(float arg1);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MyWidget *ui;
    float x;

    float alpha = 0.3;
    float beta = 1.5;
    float gamma =0.3;
    float K = 0.1;
    int K_update_period = 25;
    float point_radius = 0.1;
    float num_point_factor = 2.5;
    int stop_iterration = 10000;

    vector<vector<float>> net;
    vector<vector<float>> net_init;
    vector<vector<float>> C_init =		//各个城市的坐标数据
    {
        {100,100},
        {80,200},
        {130,220},
        {90,300},
        {110,304}
    };
    vector<vector<float>> C=		//各个城市的坐标数据
    {
        {100,100},
        {80,200},
        {130,220},
        {90,300},
        {110,304}
    };
    int n=5;//城市的数量
    int cnt=0;
    bool start=0;
};
#endif // MYWIDGET_H
