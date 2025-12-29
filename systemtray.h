#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QCoreApplication>
#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>


class SystemTray : public QObject
{
    Q_OBJECT

public:
    explicit SystemTray(QObject *parent = nullptr);
    ~SystemTray();

    // 初始化托盘（必须在 QApplication 创建后调用）
    Q_INVOKABLE void initialize();

    // 显示/隐藏托盘图标
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();

    // 设置托盘图标
    Q_INVOKABLE void setIcon(const QString &iconPath);

    // 设置提示信息
    Q_INVOKABLE void setToolTip(const QString &tip);

    // 显示托盘消息
    Q_INVOKABLE void showMessage(const QString &title, const QString &message, int durationMs = 3000);

signals:
    // 托盘图标被点击
    void trayIconClicked();

    // 菜单项被点击
    void showMainWindowRequested();
    void showSettingsRequested();
    void changeWallpaperRequested();
    void quitApplicationRequested();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onShowSettings();
    void onChangeWallpaper();
    void onQuitApplication();

private:
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;

    // 菜单项
    QAction *m_showMainWindowAction;
    QAction *m_showSettingsAction;
    QAction *m_changeWallpaperAction;
    QAction *m_quitAction;
    bool m_initialized;

    void createTrayMenu();
};

#endif // SYSTEMTRAY_H
