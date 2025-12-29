#include "utils.h"
#include <QGuiApplication>
#include <QScreen>

Utils::Utils() {}
Utils::~Utils() {}

void Utils::get_screen_size()
{
    // 获取主屏幕
    QScreen *screen = QGuiApplication::primaryScreen();

    // 获取屏幕完整尺寸（包括任务栏区域）
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width();
    int height = screenGeometry.height();


    this->m_screen_width = width;
    this->m_screen_height = height;

}

int Utils::get_width()
{
    return this->m_screen_width;
}

int Utils::get_heigth()
{
    return this->m_screen_height;
}
