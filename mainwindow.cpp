#include "mainwindow.h"
#include "ui_mainwindow.h"


// Boost Library
//#include <boost/filesystem.hpp>
//#include <boost/range/iterator_range.hpp>

// OpenCV Library
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/video/video.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/video/tracking.hpp>

// C++ Library
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <string>
#include <cstdlib>


// Namespaces
using namespace boost::filesystem;
using namespace std;
using namespace cv;


inline QImage  cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
        static QVector<QRgb>  sColorTable;

        // only create our color table once

        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );
        return image;
    }

    default:
    {
       break;
    }

    }

    return QImage();
}

inline QPixmap cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->fps = 5;

    this->readStationFile(this->station1File, this->station1Data);
    this->readStationFile(this->station2File, this->station2Data);

    this->readStationFile(this->stat1File, this->stat1Data);
    this->readStationFile(this->stat2File, this->stat2Data);

    // DEBUG:
    std::cout << this->station1Data.size() << std::endl;
    std::cout << this->station2Data.size() << std::endl;

    this->calcWaitTime(this->stat1Data, this->avgWait1);
    this->calcWaitTime(this->stat2Data, this->avgWait2);

//    this->computeAvgWaitTime(this->stat1Data, this->avgWait1);
//    this->computeAvgWaitTime(this->stat2Data, this->avgWait2);

    // Debug: display avg waiting time
//    vector<int>::iterator wtItr = this->avgWait2.begin();
//    while(wtItr != this->avgWait2.end()){
//        std::cout << *wtItr++ << std::endl;
//    }



    this->readPositionFile(this->positionFile, this->position);
    this->listFrameFilePaths();
    this->frameNumber = 0;

    this->station1AlertLevel = (this->fps * std::stoi(ui->lineEdit->text().toStdString()));
    this->station1AlarmLevel = (this->fps * std::stoi(ui->lineEdit_2->text().toStdString()));
    this->station2AlertLevel = (this->fps * std::stoi(ui->lineEdit_3->text().toStdString()));
    this->station2AlarmLevel = (this->fps * std::stoi(ui->lineEdit_4->text().toStdString()));
    this->personnelAlertLevel = (this->fps * std::stoi(ui->lineEdit_5->text().toStdString()));
    this->personnelAlarmLevel = (this->fps * std::stoi(ui->lineEdit_6->text().toStdString()));

    this->setWorkerColors();

    this->computeWorkerTime();
    this->computeWaitTime();



    this->numCols = 3;
    this->minFrameNumber = 1;
    this->maxFrameNumber = 1940;

    // Debug worker time compute
//    map<int,int>::iterator mIter = this->workerTime.begin();
//    while(mIter != this->workerTime.end()){
//        std::cout << mIter->first << " : " << mIter->second << std::endl;
//        mIter++;
//    }

//     Debug worker waiting time
    vector<map<int,int>>::iterator wIter = this->waitTime.begin();
    while(wIter != this->waitTime.end()){
        map<int,int>::iterator wmIter = wIter->begin();
        while(wmIter != wIter->end()){
            std::cout << wmIter->first << ":" << wmIter->second << " ";
            wmIter++;
        }
        std::cout << std::endl;
        wIter++;
    }

    this->computeTotalInRoomTime();
//    this->displayInRoomTime();
    this->computeAvgRPIWaitTime();

    // Debug:: Display avg rpi wait time
//    vector<int>::iterator rpiwItr = this->avgRPIWait.begin();
//    while( rpiwItr != this->avgRPIWait.end()){
//        std::cout << *rpiwItr++ << std::endl;
//    }

    ui->label->setStyleSheet("border: 5px solid black;"
                             "border-radius: 10px;");
    ui->label->setPixmap(QPixmap(":/PaloVerde.jpg"));
    ui->label->setScaledContents(true);
    ui->label->show();

    ui->label_2->setStyleSheet("border: 5px solid black;"
                             "border-radius: 10px;"
                               "background-color: cyan;");
    ui->label_2->setPixmap(QPixmap(":/RadIsland.png"));
    ui->label_2->setScaledContents(true);
    ui->label_2->show();


    ui->label_11->setText(QString::number(0));
    ui->label_12->setText(QString::number(0));
    ui->label_27->setText(QString::number(0));



    ui->label_14->setText(QString("Controls"));
    ui->label_15->setText(QString("Monitor"));
    ui->label_16->setText(QString("Status"));
    ui->label_33->setText(QString("Status"));

    ui->label_14->setAlignment(Qt::AlignCenter);
    ui->label_15->setAlignment(Qt::AlignCenter);
    ui->label_16->setAlignment(Qt::AlignCenter);

    ui->label_33->setAlignment(Qt::AlignCenter);

    ui->tableWidget->setStyleSheet("font: bold 12px;");
    ui->tableWidget_2->setStyleSheet("font: bold 12px;");
    ui->tableWidget_3->setStyleSheet("font: bold 12px;");

    ui->tableWidget->setColumnCount(this->numCols);
    ui->tableWidget->setRowCount(3);
    ui->tableWidget->setHorizontalHeaderLabels({"ID # ", "time\n @Station", "Total \n time"});
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView* header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);


    ui->label_6->setPixmap(QPixmap(":/greenLight.png"));
    ui->label_6->setScaledContents(true);
    ui->label_6->show();

    ui->tableWidget_2->setColumnCount(this->numCols);
    ui->tableWidget_2->setRowCount(3);
    ui->tableWidget_2->setHorizontalHeaderLabels({"ID # ", "time\n @Station", "Total \n time"});
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView* header2 = ui->tableWidget_2->horizontalHeader();
    header2->setSectionResizeMode(QHeaderView::Stretch);

    ui->label_7->setPixmap(QPixmap(":/greenLight.png"));
    ui->label_7->setScaledContents(true);
    ui->label_7->show();

    ui->tableWidget_3->setColumnCount(2);
    ui->tableWidget_3->setRowCount(6);
    ui->tableWidget_3->setHorizontalHeaderLabels({"ID # ", "time\n @RPI"});
    ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView* header3 = ui->tableWidget_3->horizontalHeader();
    header3->setSectionResizeMode(QHeaderView::Stretch);

    ui->label_8->setPixmap(QPixmap(":/greenLight.png"));
    ui->label_8->setScaledContents(true);
    ui->label_8->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked(){

    this->station1AlertLevel = (this->fps * std::stoi(ui->lineEdit->text().toStdString()));
    this->station1AlarmLevel = (this->fps * std::stoi(ui->lineEdit_2->text().toStdString()));
    this->station2AlertLevel = (this->fps * std::stoi(ui->lineEdit_3->text().toStdString()));
    this->station2AlarmLevel = (this->fps * std::stoi(ui->lineEdit_4->text().toStdString()));
    this->personnelAlertLevel = (this->fps * std::stoi(ui->lineEdit_5->text().toStdString()));
    this->personnelAlarmLevel = (this->fps * std::stoi(ui->lineEdit_6->text().toStdString()));
    this->rpiAlertLevel = (this->fps * std::stoi(ui->lineEdit_7->text().toStdString()));
    this->rpiAlarmLevel = (this->fps * std::stoi(ui->lineEdit_8->text().toStdString()));

    this->computeTotalInRoomTime();
    this->computeAvgRPIWaitTime();

//    this->computeAvgWaitTime(this->stat1Data, this->avgWait1);
//    this->computeAvgWaitTime(this->stat2Data, this->avgWait2);


    for(int i = this->minFrameNumber; i < this->maxFrameNumber; i++){
        this->frameNumber = i;
        string filePath = this->filePaths[this->frameNumber];
        this->videoFrame = cv::imread(filePath, 1);
        this->qVideoFrame = cvMatToQPixmap(this->videoFrame);
        
        // display the video
        ui->label->setPixmap(this->qVideoFrame);
        ui->label->setScaledContents(true);
        ui->label->show();
        ui->label->repaint();

        // display the position of workers
        int m = 1; //10
        // plot the positions of the workers
        cv::Mat island = cv::imread("/Users/ajamgard.1/Desktop/DOE/QtProjects/NEUPv2/RadIsland.png");
        cv::Mat posBkgnd;
//        cv::resize(island, posBkgnd, cv::Size(16*m, 45*m), 0, 0, 1);
        posBkgnd  = island;
        int height = island.size().height;
        for(int w = 0; w < 3; w++){
            int wid = static_cast<int>(this->position[this->frameNumber][3*w]);
            float x = m * this->position[this->frameNumber][3*w+1];
            float y = m * this->position[this->frameNumber][3*w+2];
            if(x != -1.0 && y != -1.0){
                std::cout << "Circle: " << x << ", " << y << std::endl;
                cv::circle(posBkgnd, cv::Point(x-10*m+10,height - y), 10, cv::Scalar(this->workerColors[wid][2], this->workerColors[wid][1], this->workerColors[wid][0]), -1);
                cv::putText(posBkgnd, std::to_string(wid), cv::Point(x-10*m+10,height - y), FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0,0,0), 1.0 );
            }
        }
        ui->label_2->setPixmap(cvMatToQPixmap(posBkgnd));
        ui->label_2->setScaledContents(true);
        ui->label_2->show();
// --------------------------------------------------------------------------------------------

        vector<int> stat1 = this->stat1Data[this->frameNumber];
        vector<int> stat2 = this->stat2Data[this->frameNumber];

        // update Station 1
        int nWorker = 0;
        int sStatus1 = 0;
        if(stat1.size() > 0){
            int r = 0;
            nWorker = stat1.size()/2;
            for(int j = 0; j < nWorker; j++){
                int id = stat1[2*j];
                int wTime = stat1[2*j+1];
                int status = 0;
                if(wTime > this->personnelAlarmLevel && status < 2){
                    status = 2;
                }
                if(wTime > this->personnelAlertLevel && status < 1){
                    status = 1;
                }
                if(wTime>0){
                    QTableWidgetItem* idItm = new QTableWidgetItem(QString::number(id));
                    QTableWidgetItem* wTimeItm = new QTableWidgetItem(QString::number(wTime/this->fps));

                    idItm->setTextAlignment(Qt::AlignCenter);
                    wTimeItm->setTextAlignment(Qt::AlignCenter);
                    idItm->setBackground(QColor(this->workerColors[id][0], this->workerColors[id][1], this->workerColors[id][2]));

                    QTableWidgetItem* tTimeItm = new QTableWidgetItem(QString::number(this->waitTime[this->frameNumber][id] / this->fps));
                    tTimeItm->setTextAlignment(Qt::AlignCenter);
                    ui->tableWidget->setItem(r,2,tTimeItm);

                    if(status==1){
                        wTimeItm->setBackground(Qt::yellow);
                    } else if(status==2) {
                        wTimeItm->setBackground(Qt::red);
                    } else {
                        wTimeItm->setBackground(Qt::green);
                    }
                    ui->tableWidget->setItem(r,0,idItm);
                    ui->tableWidget->setItem(r,1, wTimeItm);

                    r++;
                }
            }
        }
        // update station1 status
        if(this->avgWait1[this->frameNumber] > this->station1AlertLevel){
            sStatus1 = 1;
        } else {
            sStatus1 = 0;
        }
        if(this->avgWait1[this->frameNumber] > this->station1AlarmLevel){
            sStatus1 = 2;
        }

        if(sStatus1 == 1){
            ui->label_6->setPixmap(QPixmap(":/warning.png"));
            ui->label_6->setScaledContents(true);
            ui->label_6->show();
        } else if(sStatus1 == 2){
            ui->label_6->setPixmap(QPixmap(":/alarm.jpg"));
            ui->label_6->setScaledContents(true);
            ui->label_6->show();
        } else {
            ui->label_6->setPixmap(QPixmap(":/greenLight.png"));
            ui->label_6->setScaledContents(true);
            ui->label_6->show();
        }
        ui->label_11->setText(QString::number(this->avgWait1[i]/ this->fps));


        // Update Station 2
        int sStatus2 = 0 ;
        nWorker = 0;
        if(stat2.size() > 0){
            int r = 0;
            nWorker = stat2.size()/2;
            for(int j = 0; j < nWorker; j++){
                int id = stat2[2*j];
                int wTime = stat2[2*j+1];
                int status = 0;

                if(wTime > this->personnelAlarmLevel && status < 2){
                    status = 2;
                }
                if(wTime > this->personnelAlertLevel && status < 1){
                    status = 1;
                }
                if(wTime>0){
                    QTableWidgetItem* idItm = new QTableWidgetItem(QString::number(id));
                    QTableWidgetItem* wTimeItm = new QTableWidgetItem(QString::number(wTime/this->fps));
                    idItm->setTextAlignment(Qt::AlignCenter);
                    wTimeItm->setTextAlignment(Qt::AlignCenter);

                    idItm->setBackground(QColor(this->workerColors[id][0], this->workerColors[id][1], this->workerColors[id][2]));
                    QTableWidgetItem* tTimeItm = new QTableWidgetItem(QString::number(this->waitTime[this->frameNumber][id] / this->fps));
                    tTimeItm->setTextAlignment(Qt::AlignCenter);
                    ui->tableWidget_2->setItem(r,2,tTimeItm);

                    if(status==1){
                        wTimeItm->setBackground(Qt::yellow);
                    } else if(status==2) {
                        wTimeItm->setBackground(Qt::red);
                    } else {
                        wTimeItm->setBackground(Qt::green);
                    }

                    ui->tableWidget_2->setItem(r,0,idItm);
                    ui->tableWidget_2->setItem(r,1, wTimeItm);
                    r++;
                }
            }
        }
        // update station2 status
        if(this->avgWait2[this->frameNumber] > this->station2AlertLevel){
            sStatus2 = 1;
        } else {
            sStatus2 = 0;
        }
        if(this->avgWait2[this->frameNumber] > this->station2AlarmLevel){
            sStatus2 = 2;
        }

        if(sStatus2 == 1){
            ui->label_7->setPixmap(QPixmap(":/warning.png"));
        } else if(sStatus2 == 2){
            ui->label_7->setPixmap(QPixmap(":/alarm.jpg"));
        } else {
            ui->label_7->setPixmap(QPixmap(":/greenLight.png"));
        }
        ui->label_7->setScaledContents(true);
        ui->label_7->show();
        ui->label_12->setText(QString::number(this->avgWait2[i]/ this->fps));

        // Update RPI
        int rpiStatus = 0;
        map<int,int> workerTime = this->inRoomTime[this->frameNumber];
        nWorker = workerTime.size();
        int r = 0;
        map<int,int>::iterator wItr = workerTime.begin();
        for(; wItr != workerTime.end(); wItr++){
            int id = wItr->first;
            int wTime = wItr->second;
            int status = 0;
            if(wTime > this->rpiAlarmLevel && status < 2){
                status = 2;
            }
            if(wTime > this->rpiAlertLevel && status < 1){
                status = 1;
            }
            if(wTime > 0){
                // display worker time in table
                QTableWidgetItem* idItm = new QTableWidgetItem(QString::number(id));
                QTableWidgetItem* wTimeItm = new QTableWidgetItem(QString::number(wTime/this->fps));
                idItm->setTextAlignment(Qt::AlignCenter);
                wTimeItm->setTextAlignment(Qt::AlignCenter);
                idItm->setBackground(QColor(this->workerColors[id][0], this->workerColors[id][1], this->workerColors[id][2]));
                if(status==1){
                    wTimeItm->setBackground(Qt::yellow);
                } else if(status==2) {
                    wTimeItm->setBackground(Qt::red);
                } else {
                    wTimeItm->setBackground(Qt::green);
                }

                ui->tableWidget_3->setItem(r,0,idItm);
                ui->tableWidget_3->setItem(r,1, wTimeItm);
                r++;
            }
        }
        // update rpi status
        int rpiWaitTime = this->avgRPIWait[this->frameNumber];
        std::cout << this->frameNumber << ":" << rpiWaitTime;
        if(rpiWaitTime > this->rpiAlertLevel){
            rpiStatus = 1;
        } else {
            rpiStatus = 0;
        }
        if(rpiWaitTime > this->rpiAlarmLevel){
            rpiStatus = 2;
        }

        if(rpiStatus == 1){
            ui->label_8->setPixmap(QPixmap(":/warning.png"));
        } else if(rpiStatus == 2){
            ui->label_8->setPixmap(QPixmap(":/alarm.jpg"));
        } else {
            ui->label_8->setPixmap(QPixmap(":/greenLight.png"));
        }
        ui->label_8->setScaledContents(true);
        ui->label_8->show();
        ui->label_27->setText(QString::number(rpiWaitTime/this->fps));

        // save gui screenshot to imagefile
        std::stringstream ss;
        ss << setw(4) << setfill('0') << this->frameNumber;
        std::string guiSceenFileName = this->guiSceenShotDir + ss.str() + ".png";
        this->grab().save(QString(guiSceenFileName.c_str()), "png");
        std::cout << guiSceenFileName << std::endl;

        // wipe table1 and table2 clean
        for(int m = 0; m < 3; m++){
            for(int n = 0; n < this->numCols; n++){
                ui->tableWidget->setItem(m,n,new QTableWidgetItem());
                ui->tableWidget_2->setItem(m,n,new QTableWidgetItem());
            }
        }

        // wipe table3 clean
        for(int m = 0; m < 6; m++){
            for(int n = 0; n < 2; n++){
                ui->tableWidget_3->setItem(m,n, new QTableWidgetItem());
            }
        }
    }
}

void MainWindow::on_pushButton_2_clicked(){
    this->close();
}
