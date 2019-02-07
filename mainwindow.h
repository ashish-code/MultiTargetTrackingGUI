#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QThread>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QColormap>
#include <QtCore>
#include <QtGui>
#include <QPainter>
#include <QPixmap>
#include <QBrush>
#include <QImage>

// Boost Library
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

// OpenCV Library
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

// C++ Library
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <string>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <map>

// Namespaces
using namespace boost::filesystem;
using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    // members
    std::string station1File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/station1File.txt";
    std::string station2File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/station2File.txt";
//    std::string station3File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/station3File.txt";
    std::string positionFile = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/positionFile.txt";
    std::string guiSceenShotDir = "/Users/ajamgard.1/Desktop/DOE/GuiScreen/";
    std::string frameDir = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/Frames/";

    std::string stat1File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/stat1File.txt";
    std::string stat2File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/stat2File.txt";
//    std::string stat3File = "/Users/ajamgard.1/Desktop/DOE/NEUPv2/stat3File.txt";

    vector<vector<int>> station1Data;
    vector<vector<int>> station2Data;
//    vector<vector<int>> station3Data;

    vector<vector<int>> stat1Data;
    vector<vector<int>> stat2Data;
//    vector<vector<int>> stat3Data;

    vector<int> avgWait1;
    vector<int> avgWait2;
    vector<int> avgRPIWait;

    vector<map<int,int>> inRoomTime;

    std::vector<std::string> filePaths;
    vector<vector<float>> position;
    vector<vector<int>> workerColors;
    map<int,int> workerTime;
    vector<map<int,int>> waitTime;

    int numCols;
    int minFrameNumber;
    int maxFrameNumber;

    cv::Mat videoFrame;
    QPixmap qVideoFrame;
    int frameNumber;
    int fps;


    // methods
    void listFrameFilePaths(){
        boost::filesystem::path p(frameDir);
        if(is_directory(p)) {
            boost::filesystem::directory_iterator iter{p};
            for(;iter != boost::filesystem::directory_iterator{}; iter++){
                filePaths.push_back(iter->path().string());    
            }
            std::sort(filePaths.begin(),filePaths.end());
        }
        return;
    }

    void computeAvgWaitTime(vector<vector<int>>& data, vector<int>& time){
        int n = data.size();
        int avgTime = 0;
        time.push_back(avgTime);
        int nWorkerExited = 0;
        for(int i = 1; i < n; i++){
            time.push_back(avgTime);
            if(data[i].size() == 0){
                // person left and queue is empty
                if(data[i-1].size() != 0){
                    avgTime = nWorkerExited*avgTime + data[i-1][1];
                    nWorkerExited++;
                    avgTime /= nWorkerExited;
                    std::cout << avgTime << " : " << nWorkerExited << std::endl;
                } else {
                    continue;
                }
            } else if(data[i][0] != data[i-1][0]){
                // person left and queue is not empty
                avgTime = nWorkerExited*avgTime + data[i-1][1];
                nWorkerExited++;
                avgTime /= nWorkerExited;
            } else {
                continue;
            }
        }
    }

    void computeAvgRPIWaitTime(){
        unsigned long i = 0;
        unsigned long j = i+1;
        int avgTime = 0;
        int nLeft = 0;
        this->avgRPIWait.push_back(avgTime);
        for(; i < this->inRoomTime.size()-1; i++, j++){
            // for each map when the curr is -1 and prev is not -1, update avg time
            map<int,int> prev = this->inRoomTime[i];
            map<int,int> curr = this->inRoomTime[j];
            map<int,int>::iterator pItr = prev.begin();
            map<int,int>::iterator cItr = curr.begin();
            for(; pItr != prev.end() && cItr != curr.end(); pItr++, cItr++){
                if(cItr->second == -1 && pItr->second !=-1){

                    avgTime = nLeft* avgTime + pItr->second;
                    nLeft++;
                    avgTime /= nLeft;
                }
            }
            this->avgRPIWait.push_back(avgTime);
        }
    }

    void calcWaitTime(vector<vector<int>>& data, vector<int>& time){
        unsigned long i = 0;
        unsigned long j = i+1;
        int avgTime = 0;
        int nLeft = 0;
        time.push_back(avgTime);
        for(; i < data.size(); i++, j++){
            // prev and curr are empty
            if(data[i].size() ==0 && data[j].size() == 0){
                time.push_back(avgTime);
            }
            // prev is empty and curr is not
            if(data[i].size() == 0 && data[j].size() != 0){
                // new person in an empty queue
                time.push_back(avgTime);
            }
            // curr is empty and prev is not
            if(data[j].size() == 0 && data[i].size() != 0){
                // last person left the queue

                avgTime = nLeft*avgTime + data[i][1];
                nLeft++;
                avgTime /= nLeft;
                time.push_back(avgTime);
            }
            // prev and curr are not empty
            if(data[i].size() != 0 && data[j].size() != 0){
                // prev and curr are same
                if(data[i][0] == data[j][0]){
                    time.push_back(avgTime);
                } else {
                    // prev and curr are different

                    avgTime = nLeft* avgTime + data[i][1];
                    nLeft++;
                    avgTime /= nLeft;
                    time.push_back(avgTime);
                }
            }

        }
    }


    void readStationFile(std::string& fileName, vector<vector<int>>& fields){
        std::ifstream in(fileName);
        if (in) {
            string line;
            while (getline(in, line)) {
                fields.push_back(vector<int>());
                if(line.empty()){
                    continue;
                } else {
                    stringstream sep(line);
                    string field;
                    while (getline(sep, field, ',')) {
                        fields.back().push_back(stoi(field));
                    }
                }

            }
            in.close();
        }
        return;
    }

    void readPositionFile(std::string& fileName, vector<vector<float>>& fields){
        std::ifstream in(fileName);
        if(in){
            string line;
            while(getline(in, line)){
                stringstream sep(line);
                string field;
                fields.push_back(vector<float>());
                while(getline(sep, field, ',')){
                    fields.back().push_back(stof(field));
                }
            }
            in.close();
        }
        return;
    }

    void setWorkerColors(){
        std::srand(std::time(NULL));
        for(int i = 0; i < 10; i++){
            vector<int> v = {127+rand()%127, 127+rand()%127, 127+rand()%127};
            workerColors.push_back(v);
        }
    }

    void computeWorkerTime(){
        if(position.empty()){
            readPositionFile(positionFile, position);
        }
        vector<vector<float>>::iterator i = position.begin();
        for(;i!=position.end();i++){
            vector<float>::iterator j = i->begin();
            while(j != i->end()){
                int id = static_cast<int>(*j++);
                float x = *j++;
                float y = *j++;
                std::cout << "id: " << id << " ,x: "<< x << " ,y: "<< y << std::endl;
                if(x > 0.0 && y > 0.0){
                    workerTime[id]++;
                }
            }

        }
    }

    void computeWaitTime(){
        if(position.empty()){
            readPositionFile(positionFile, position);
        }
        vector<map<int,int>> wTime;
        vector<vector<float>>::iterator i = position.begin();
        for(;i!=position.end();i++){
            map<int,int> currTime;
            vector<float>::iterator j = i->begin();
            while(j != i->end()){
                int id = static_cast<int>(*j++);
                float x = *j++;
                float y = *j++;
                if(x > 0.0 && y > 0.0){
                    currTime[id] = 1;
                } else {
                    currTime[id] = 0;
                }
            }
            wTime.push_back(currTime);
        }
        // cummulative sum
        vector<map<int,int>>::iterator itr = wTime.begin();
        map<int,int> prev = *itr;
        waitTime.push_back(prev);
        itr += 1; // boundary value, first row is unchanged
        while(itr != wTime.end()){
            map<int,int> curr = *itr;
            map<int,int>::iterator iter = curr.begin();
            while(iter != curr.end()){
                prev[iter->first] += curr[iter->first];
                iter++;
            }
            waitTime.push_back(prev);
            itr++;
        }
    }

    void computeTotalInRoomTime(){
        if(position.empty()){
            readPositionFile(positionFile, position);
        }
        vector<map<int,int>> wTime;
        vector<vector<float>>::iterator i = position.begin();
        for(;i!=position.end();i++){
            map<int,int> currTime;
            vector<float>::iterator j = i->begin();
            while(j != i->end()){
                int id = static_cast<int>(*j++);
                float x = *j++;
                float y = *j++;
                if(x > 0.0 || y > 0.0){
                    currTime[id] = 1;
                } else {
                    currTime[id] = -1;
                }
            }
            wTime.push_back(currTime);
        }
        // cummulative sum
        vector<map<int,int>>::iterator itr = wTime.begin();
        map<int,int> prev = *itr;
        inRoomTime.push_back(prev);
        itr += 1; // boundary value, first row is unchanged
        while(itr != wTime.end()){
            map<int,int> curr = *itr;
            map<int,int>::iterator iter = curr.begin();
            while(iter != curr.end()){
                if(curr[iter->first] == -1){
                    prev[iter->first] = -1;
                } else {
                    prev[iter->first] += curr[iter->first];
                }
                iter++;
            }
            inRoomTime.push_back(prev);
            itr++;
        }

    }

    void displayInRoomTime(){
        if(inRoomTime.empty()){
            computeTotalInRoomTime();
        }
        vector<map<int,int>>::iterator itr1 = inRoomTime.begin();
        while(itr1 != inRoomTime.end()){
            map<int,int> atTime = *itr1++;
            map<int,int>::iterator itr2 = atTime.begin();
            while(itr2 != atTime.end()){
                std::cout << itr2->first << ":" << itr2->second << ",";
                itr2++;
            }
            std::cout << std::endl;
        }
    }




private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();



private:
    Ui::MainWindow *ui;
    int personnelAlertLevel;
    int personnelAlarmLevel;
    int station1AlertLevel;
    int station1AlarmLevel;
    int station2AlertLevel;
    int station2AlarmLevel;
    int rpiAlertLevel;
    int rpiAlarmLevel;

    void displayFilePaths(){
        vector<std::string>::iterator itr = filePaths.begin();
        while(itr!=filePaths.end()){
            std::cout << *itr++ << std::endl;
        }
    }
};


#endif // MAINWINDOW_H
