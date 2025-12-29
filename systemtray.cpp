#include "systemtray.h"
#include <QDebug>
#include <QIcon>
#include <QCoreApplication>
#include <QApplication>

SystemTray::SystemTray(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_showMainWindowAction(nullptr)
    , m_showSettingsAction(nullptr)
    , m_changeWallpaperAction(nullptr)
    , m_quitAction(nullptr)
    , m_initialized(false)
{
    qDebug() << "SystemTray constructor called";
}

void SystemTray::initialize()
{
    if (m_initialized) {
        qWarning() << "SystemTray already initialized";
        return;
    }

    qDebug() << "Initializing SystemTray";

    // 创建托盘图标和菜单
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayMenu = new QMenu();

    // 创建托盘菜单
    createTrayMenu();

    // 设置托盘菜单
    m_trayIcon->setContextMenu(m_trayMenu);

    // 连接托盘图标激活信号
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &SystemTray::onTrayIconActivated);

    // 设置默认图标（使用应用图标）
    setIcon(":/icon/app_icon.png");

    // 设置默认提示
    setToolTip("Pap.er - 壁纸管理");

    // 检查系统是否支持托盘
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray is not available on this system";
    }

    m_initialized = true;
    qDebug() << "SystemTray initialized successfully";
}

SystemTray::~SystemTray()
{
    // 清理菜单（QSystemTrayIcon 会自动处理，但手动清理更安全）
    if (m_trayMenu) {
        delete m_trayMenu;
        m_trayMenu = nullptr;
    }
}

void SystemTray::show()
{
    if (!m_initialized) {
        qWarning() << "SystemTray not initialized, call initialize() first";
        return;
    }

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon->show();
        qDebug() << "System tray icon shown";
    } else {
        qWarning() << "Cannot show system tray - not available";
    }
}

void SystemTray::hide()
{
    if (!m_initialized) {
        return;
    }

    m_trayIcon->hide();
    qDebug() << "System tray icon hidden";
}

void SystemTray::setIcon(const QString &iconPath)
{
    if (!m_initialized) {
        qWarning() << "SystemTray not initialized";
        return;
    }
    QIcon icon(iconPath);

    // 如果指定的图标不存在，使用默认图标
    if (icon.isNull()) {
        qWarning() << "Icon not found:" << iconPath << "- using default";
        // 使用应用图标
        icon = QIcon(":/icon/app_icon.png");

        // 如果还是没有，使用系统图标
        if (icon.isNull()) {
            icon = QIcon::fromTheme("applications-graphics");
        }
    }

    m_trayIcon->setIcon(icon);
    qDebug() << "Tray icon set to:" << iconPath;
}

void SystemTray::setToolTip(const QString &tip)
{
    if (!m_initialized) {
        return;
    }
    m_trayIcon->setToolTip(tip);
}

void SystemTray::showMessage(const QString &title, const QString &message, int durationMs)
{
    if (!m_initialized) {
        qWarning() << "SystemTray not initialized";
        return;
    }
    m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, durationMs);
    qDebug() << "Tray message shown:" << title << "-" << message;
}

void SystemTray::createTrayMenu()
{
    // 显示主窗口
    m_showMainWindowAction = new QAction("显示主窗口", this);
    m_showMainWindowAction->setIcon(QIcon::fromTheme("window"));
    connect(m_showMainWindowAction, &QAction::triggered,
            this, &SystemTray::onShowMainWindow);

    // 设置
    m_showSettingsAction = new QAction("设置", this);
    m_showSettingsAction->setIcon(QIcon::fromTheme("preferences-system"));
    connect(m_showSettingsAction, &QAction::triggered,
            this, &SystemTray::onShowSettings);

    // 立即更换壁纸
    m_changeWallpaperAction = new QAction("立即更换壁纸", this);
    m_changeWallpaperAction->setIcon(QIcon::fromTheme("view-refresh"));
    connect(m_changeWallpaperAction, &QAction::triggered,
            this, &SystemTray::onChangeWallpaper);

    // 退出
    m_quitAction = new QAction("退出", this);
    m_quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(m_quitAction, &QAction::triggered,
            this, &SystemTray::onQuitApplication);

    // 添加到菜单
    m_trayMenu->addAction(m_showMainWindowAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_showSettingsAction);
    m_trayMenu->addAction(m_changeWallpaperAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);

    qDebug() << "Tray menu created";
}

void SystemTray::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        // 单击托盘图标
        qDebug() << "Tray icon clicked";
        emit trayIconClicked();
        emit showMainWindowRequested();
        break;

    case QSystemTrayIcon::DoubleClick:
        // 双击托盘图标
        qDebug() << "Tray icon double-clicked";
        emit showMainWindowRequested();
        break;

    case QSystemTrayIcon::MiddleClick:
        // 中键点击
        qDebug() << "Tray icon middle-clicked";
        emit changeWallpaperRequested();
        break;

    default:
        break;
    }
}

void SystemTray::onShowMainWindow()
{
    qDebug() << "Show main window requested from tray";
    emit showMainWindowRequested();
}

void SystemTray::onShowSettings()
{
    qDebug() << "Show settings requested from tray";
    emit showSettingsRequested();
}

void SystemTray::onChangeWallpaper()
{
    qDebug() << "Change wallpaper requested from tray";
    emit changeWallpaperRequested();
}

void SystemTray::onQuitApplication()
{
    qDebug() << "Quit application requested from tray";
    emit quitApplicationRequested();
}
