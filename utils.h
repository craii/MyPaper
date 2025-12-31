#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QGuiApplication>
#include <QScreen>
#include <QFileInfo>

class Utils:public QObject
{
    Q_OBJECT
    Q_PROPERTY(int screen_width READ screen_width CONSTANT)
    Q_PROPERTY(int screen_height READ screen_height CONSTANT)

public:
    Utils();
    ~Utils();
    int screen_width() const{return m_screen_width;}
    int screen_height() const{return m_screen_height;}
    void get_screen_size();
    int get_width();
    int get_heigth();
    bool hasPermission(const QString &path);


private:
    int m_screen_width;
    int m_screen_height;


};

#endif // UTILS_H
