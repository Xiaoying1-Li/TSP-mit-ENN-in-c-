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
    void iterration();
    vector<F32vec4> circle_generate(float center_x,float center_y);
    vector<F32vec4> normalize(vector<F32vec4> C);
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

    vector<F32vec4> net;
    vector<F32vec4> net_init;
    vector<F32vec4> C_init =		//各个城市的坐标数据
    {
		F32vec4(100,100,0,0),
		F32vec4(80,200,0,0),
		F32vec4(130,220,0,0),
		F32vec4(90,300,0,0),
		F32vec4(110,304,0,0)
    };
    vector<F32vec4> C=		//各个城市的坐标数据
    {
		F32vec4(100,100,0,0),
		F32vec4(80,200,0,0),
		F32vec4(130,220,0,0),
		F32vec4(90,300,0,0),
		F32vec4(110,304,0,0)
    };
    int n=5;//城市的数量
    int cnt=0;
    bool start=0;
};
#endif // MYWIDGET_H
