#-------------------------------------------------
#
# Project created by QtCreator 2017-02-10T11:51:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NEUP
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    posedetection.cpp \
    humantracking.cpp \
    transformations.cpp

HEADERS  += mainwindow.h \
    posedetection.h \
    humantracking.h \
    transformations.h

FORMS    += mainwindow.ui

# if LINUX
#INCLUDEPATH += /usr/local/include/

#LIBS += -L/usr/lib/x86-64_linux_gnu/
#LIBS += -lopencv_core
#LIBS += -lopencv_highgui
#LIBS += -lopencv_video
#LIBS += -lopencv_contrib
#LIBS += -lopencv_objdetect
#LIBS += -lopencv_imgproc
#LIBS += -lopencv_videoio
#LIBS += -lopencv_imgcodecs

#LIBS += -L/usr/local/lib/
#LIBS += -lboost_filesystem
#LIBS += -lboost_system
#LIBS += -lboost_container

# if MacOX

INCLUDEPATH += /usr/local/opt/opencv3/include/

LIBS += -L/usr/local/opt/opencv3/lib/
LIBS += -lopencv_core
LIBS += -lopencv_highgui
LIBS += -lopencv_video
LIBS += -lopencv_videoio
LIBS += -lopencv_imgproc
LIBS += -lopencv_imgcodecs

INCLUDEPATH += /usr/local/opt/boost/include/


LIBS += -L/usr/local/opt/boost/lib/
LIBS += -lboost_filesystem
LIBS += -lboost_system
LIBS += -lboost_container

RESOURCES += \
    images.qrc
