TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt



unix{
INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so

LIBS += /usr/local/ffmpeg/lib/libavdevice.so

LIBS += /usr/local/ffmpeg/lib/libavcodec.so

LIBS += /usr/local/ffmpeg/lib/libavutil.so

LIBS += /usr/local/ffmpeg/lib/libswscale.so

LIBS += /usr/local/ffmpeg/lib/libswresample.so
}

SOURCES += \
        main.cpp
