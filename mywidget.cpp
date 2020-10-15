#include "mywidget.h"
#include "ui_mywidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <vector>
#include <cstdio>
#include <iostream>
#include <windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <QFileDialog>
#include <QDebug>
#include <sstream>
#include <string>
#include <algorithm>
#include <QTimer>
#include <QElapsedTimer>

using namespace std;

#define pi 3.14159265


vector<F32vec4> MyWidget::circle_generate(float center_x,float center_y) {//向量化计算
    int num_points = (int)(num_point_factor * n);
    vector<F32vec4> net;
    for (int i = 0;i < num_points;i++) {
		F32vec4 temp(center_x + point_radius * cos(2 * i * pi / num_points), center_y + point_radius * sin(2 * i * pi / num_points),0,0);
        net.push_back(temp);
    }
    return net;//圆周上等距分割点的坐标矩阵
}
vector<F32vec4> MyWidget::normalize(vector<F32vec4> C) {//SIMD向量化计算
    float min_x = 100000;
    float max_x = -100000;
    float min_y = 100000;
    float max_y = -100000;
    for (int i = 0;i < C.size();i++) {
        cout << i << endl;
        if (C[i][0]<min_x) {
            min_x = C[i][0];
        }
        if (C[i][0] > max_x) {
            max_x = C[i][0];
        }
        if (C[i][1] < min_y) {
            min_y = C[i][1];
        }
        if (C[i][1] > max_y) {
            max_y = C[i][1];
        }
    }
    #pragma omp parallel for num_threads(threads)
	F32vec4 minus(min_x, min_y,0,0); // 向量化并行运算 SIMD
	F32vec4 divide(max_x - min_x,max_y-min_y,1,1);
    for (int i = 0;i < C.size();i++) {
        //C[i][0]=(C[i][0]-min_x)/(max_x-min_x); //没有优化过的代码
        //C[i][1]=(C[i][1]-min_y)/();
		C[i] = (C[i]-minus) / divide; // 并行运算
    }
    return C;
}

//构造函数
MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyWidget)
{
    ui->setupUi(this);
    //初始化net，并把城市坐标进行缩放
    net=circle_generate(0.5,0.5);
    net_init=circle_generate(0.5,0.5);
    C=normalize(C);
    C_init=normalize(C_init);
}

MyWidget::~MyWidget()
{
    delete ui;
}


void MyWidget::iterration() {
    //cout << "ok" << endl;
    cnt+=1;
    if(cnt%K_update_period==0){
        K=max(0.01,0.99*K);
    }
    float T=2*K*K;
    vector<vector<float>> v;//v[0]为一个数组，表示第0个城市对net中每一个点的V_ia值
    #pragma omp parallel
    #pragma omp for
    for (int i = 0;i < C.size();i++) {
        vector<float> vi;
        float i_sum = 0;
        for (int j = 0;j < net.size();j++) {
			F32vec4 dis = (net[j] - net[i]) * (net[j] - net[i]);
			float distance_ij = dis[0] + dis[1];
            float v_ia = exp(-1/ T * distance_ij );
            //cout << v_ia << endl;
            i_sum += v_ia;
            vi.push_back(v_ia);
        }
        #pragma omp parallel
        #pragma omp for
        for (int j = 0;j < net.size();j++) {
            vi[j] = vi[j] / i_sum;
        }
        v.push_back(vi);
    }

    vector<vector<float>> w;//w[0]为一个数组，表示所有结点对net[0]的w_ia值
    #pragma omp parallel
    #pragma omp for
    for (int i = 0;i < net.size();i++) {
        vector<float> wi;
        float i_sum = 0;
        for (int j = 0;j < net.size();j++) {
            //float distance_ij = (net[j][0] - net[i][0]) * (net[j][0] - net[i][0]) + (net[j][1] - net[i][1]) * (net[j][1] - net[i][1]);
			F32vec4 dis = (net[j] - net[i]) * (net[j] - net[i]);
			float distance_ij = dis[0]+dis[1];
			//cout << distance_ij << endl;
            float w_ia = exp(-1/ T * distance_ij );
            //cout << v_ia << endl;
            i_sum += w_ia;
            wi.push_back(w_ia);
        }
        #pragma omp parallel
        #pragma omp for
        for (int j = 0;j < net.size();j++) {
            wi[j] = wi[j] / i_sum;
        }
        w.push_back(wi);
    }


    //遍历net中每一个结点，计算结点的新位置
    //第一步：city对net结点的作用
    #pragma omp parallel
    #pragma omp for
    for (int i = 0;i < net.size();i++) {
		F32vec4 delta(0, 0, 0, 0);
        for (int j = 0;j < C.size();j++) {
			delta+= v[j][i] * (C[j] - net[i]);
        }
        net[i] += alpha * delta;
    }
    //第二步
    #pragma omp parallel
    #pragma omp for
    for (int i = 0;i < net.size();i++) {
		F32vec4 delta(0, 0, 0, 0);
        for (int j = 0;j < net.size();j++) {
			delta += w[i][j] * (net[i] - net[j]);
        }
        net[i] += gamma * delta;
    }

    //第3步：net各点间的影响
    vector<F32vec4> net_copy;
	net_copy.assign(net.begin(), net.end());
    #pragma omp parallel
    #pragma omp for
    for (int i = 0;i < net.size();i++) {
        if(i==0){
            //net_copy[i][0] += beta * K * (net[net.size()-1][0]+net[i+1][0]-2*net[i][0]);
            //net_copy[i][1] += beta * K * (net[net.size()-1][1]+net[i+1][1]-2*net[i][1]);没用SIMD
			net_copy[i] += beta * K * (net[net.size() - 1] + net[i + 1] - 2 * net[i]);//用了SIMD
        }
        else if (i == net.size() - 1) {
           // net_copy[i][0] += beta * K * (net[i-1][0] + net[0][0] - 2 * net[i][0]);
           // net_copy[i][1] += beta * K * (net[i-1][1] + net[0][1] - 2 * net[i][1]);
			net_copy[i] += beta * K * (net[i - 1] + net[0] - 2 * net[i]);
        }
        else {
           // net_copy[i][0] += beta * K * (net[i-1][0] + net[i + 1][0] - 2 * net[i][0]);
           // net_copy[i][1] += beta * K * (net[i-1][1] + net[i + 1][1] - 2 * net[i][1]);
			net_copy[i] += beta * K * (net[i - 1] + net[i + 1] - 2 * net[i]);
        }
    }
    net.assign(net_copy.begin(),net_copy.end());

}


void MyWidget::paintEvent(QPaintEvent *){
    QPainter p(this);
    QPen pen;
    p.setBrush(Qt::white);
    p.setPen(Qt::white);
    p.drawRect(10,120,500,500);
    pen.setWidth(5);
    pen.setColor(Qt::green);
    p.setPen(pen);

    //vector<vector<float> > C_normalize=normalize(C);
    //画图时的缩放系数
    int x_scale=20;
    int y_scale=130;
    int x_range=400;
    int y_range=400;
    //画城市图
    for(int i=0;i<C.size();i++){
        p.drawPoint(x_scale+x_range*C[i][0],y_scale+y_range*C[i][1]);
    }


    pen.setWidth(5);
    pen.setColor(Qt::red);
    p.setPen(pen);
    //画net中点的图
    for(int i=0;i<net.size();i++){
        p.drawPoint(x_scale+x_range*net[i][0],y_scale+y_range*net[i][1]);
    }
    pen.setWidth(1);
    pen.setColor(Qt::black);
    p.setPen(pen);
    //画net中点的连线
    for(int i=0;i<net.size();i++){
        if(i==0){
            p.drawLine(x_scale+x_range*net[i][0],y_scale+y_range*net[i][1],
                    x_scale+x_range*net[net.size()-1][0],y_scale+y_range*net[net.size()-1][1]);
        }else{
            p.drawLine(x_scale+x_range*net[i][0],y_scale+y_range*net[i][1],
                    x_scale+x_range*net[i-1][0],y_scale+y_range*net[i-1][1]);
        }
    }

    p.end();
}




//repaint
void MyWidget::on_pushButton_clicked()
{
    start=1;
    for(int i=0;i<100;i++){
        if(cnt>stop_iterration){
            break;
        }
        iterration();

    }
    update();
}

void MyWidget::on_Alpha_SpinBox_valueChanged(float a)
{
    alpha=a;
}

void MyWidget::on_Beta_SpinBox_valueChanged(float b)
{
    beta=b;
}

void MyWidget::on_Gamma_SpinBox_valueChanged(float g)
{
    gamma=g;
}

void MyWidget::on_K_SpinBox_valueChanged(float k)
{
    K=k;
}

void MyWidget::on_pushButton_3_clicked()
{
    net.assign(net_init.begin(),net_init.end());
    C.assign(C_init.begin(),C_init.end());
    cnt=0;
    update();
}

void MyWidget::on_pushButton_4_clicked()
{

        QString OpenFile, OpenFilePath;
        QImage image;
        OpenFile = QFileDialog::getOpenFileName(this,
                                                  "Please choose an test file",
                                                  "",
                                                  "Image Files(*.txt *.tsp);;All(*.*)");
 

        QFileInfo OpenFileInfo;
        OpenFileInfo = QFileInfo(OpenFile);
        OpenFilePath = OpenFileInfo.filePath();


        //QString displayString;
        QFile file(OpenFilePath);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<<"Can't open the file!"<<endl;
        }
        C_init={};
        C={};
        while(!file.atEnd())
        {
            QByteArray line = file.readLine();
            QString str(line);

            string s=str.toStdString();
            if(s=="EOF"){
                break;
            }
            stringstream ss(s);
            float id,x,y;
            ss>>id>>x>>y;
			F32vec4 temp(x,y,0,0);
            C.push_back(temp);
            C_init.push_back(temp);

            //displayString.append(str);
        }
        n=C.size();
        net=circle_generate(0.5,0.5);
        net_init=circle_generate(0.5,0.5);
        C=normalize(C);
        C_init=normalize(C_init);
        update();

}

void MyWidget::on_pushButton_2_clicked()
{
    start=0;
}

