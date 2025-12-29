#include <QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>
#include <QDir>
#include "PhotoBackend.h"
#include "SettingsBackend.h"
#include "utils.h"
#include "systemtray.h"


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;


    // for (int i=0; i<argc; i++)
    // {

    //     qDebug() << "argc: "<< i << "argv: " << argv[i] << Qt::endl;

    // }


    // 初始化
    PhotoBackend photoBackend;
    SettingsBackend settingsBackend;
    Utils utils;
    SystemTray systemtray;

    #define APP_WIDTH 375
    #define APP_HEIGTH 667



    // 设置初始内容
    QString exeDir  = QCoreApplication::applicationDirPath();
    //qDebug() << "程序安装路径: "<< exeDir << Qt::endl;
    photoBackend.set_application_dir(exeDir);

    utils.get_screen_size();
    int screen_width = utils.get_width();
    int screen_height = utils.get_heigth();

    #ifdef Q_OS_WIN
        #define APP_POSITION_X (screen_width - (1.5 * APP_WIDTH))
        #define APP_POSITION_Y (screen_height - (100 + APP_HEIGTH))
        QString init_wallpaper_folder = photoBackend.get_application_dir() + "/myWallPapers";
    #endif

    #ifdef Q_OS_MACOS
        #define APP_POSITION_X    screen_width - (1.5 * APP_WIDTH)
        #define APP_POSITION_Y 30
        QString init_wallpaper_folder = QDir::homePath() + "/documents/myWallPapers";
    #endif

    photoBackend.SET_APP_WIDTH(APP_WIDTH);
    photoBackend.SET_APP_HEIGTH(APP_HEIGTH);
    photoBackend.SET_APP_POSITION_X(APP_POSITION_X);
    photoBackend.SET_APP_POSITION_Y(APP_POSITION_Y);

    qDebug()<<"窗口宽度： " <<photoBackend.get_APP_WIDTH() << "\n";
    qDebug()<<"窗口高度： " <<photoBackend.get_APP_HEIGTH() << "\n";
    qDebug()<<"窗口x： " <<photoBackend.get_APP_POSITION_X() << "\n";
    qDebug()<<"窗口y： " <<photoBackend.get_APP_POSITION_Y() << "\n";

    // int screen_width = utils.get_width();
    // int screen_height = utils.get_heigth();
    // qDebug() << "screen_width: "<< utils.get_width() << Qt::endl;
    // qDebug() << "screen_height: "<< utils.get_heigth() << Qt::endl;


    // qDebug() << "壁纸路径: "<< init_wallpaper_folder << Qt::endl;
    //photoBackend.set_m_photoDirectory_base(init_wallpaper_folder);


    //配置默认参数
    QVariantMap current_setting = settingsBackend.loadSettings();

    // qDebug() << "当前配置：" << current_setting["wallpaperFolder"] << "\n";
    // qDebug() << "当前配置serverUrl：" << current_setting["serverUrl"] << "\n";
    // qDebug() << "当前配置token：" << current_setting["token"] << "\n";
    // qDebug() << "当前配置updateInterval：" << current_setting["updateInterval"] << "\n";

    QString wallpaperFolder = current_setting.value("wallpaperFolder").toString();
    QString serverUrl = current_setting.value("serverUrl").toString();
    QString token = current_setting.value("token").toString();
    QString updateInterval = current_setting.value("updateInterval").toString();

    if (wallpaperFolder == "")
    {
        settingsBackend.validateAndSave(init_wallpaper_folder, serverUrl, updateInterval,  1);

        photoBackend.set_m_photoDirectory_base(init_wallpaper_folder);
    }
    else
    {
        photoBackend.set_m_photoDirectory_base(wallpaperFolder);
    }

    // 将后端对象注册到 QML 上下文
    engine.rootContext()->setContextProperty("photoBackend", &photoBackend);
    engine.rootContext()->setContextProperty("photoModel", photoBackend.photoModel());
    engine.rootContext()->setContextProperty("settingsBackend", &settingsBackend);
    engine.rootContext()->setContextProperty("systemTray", &systemtray);


    // 初始默认的 myWallPapers 目录
    QString default_wallpaper_path;
    if (wallpaperFolder == ""){
        default_wallpaper_path = init_wallpaper_folder + "/hotest";
    }
    else{
        default_wallpaper_path = wallpaperFolder + "/hotest";
    }

    photoBackend.setPhotoDirectory(default_wallpaper_path);

    // 加载 QML
    const QUrl url(QStringLiteral("qrc:/qt/qml/wallpaper/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    engine.load(url);


    // 显示系统托盘图标
    #ifdef Q_OS_WIN

        QString icon_path = photoBackend.get_application_dir() + "/app_icon.png";

    #endif
    #ifdef Q_OS_MACOS

        QString icon_path = QDir::homePath() +"/applications/myWallPaper/resources/app_icon.png";

    #endif


    systemtray.initialize();
    systemtray.setIcon(icon_path);
    systemtray.show();


    //确保壁纸加载成功
    QTimer::singleShot(0, [&photoBackend, default_wallpaper_path]() {
        photoBackend.setPhotoDirectory(default_wallpaper_path);
    });


    //去除系统顶栏
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (window) {
        window->setFlags(Qt::FramelessWindowHint);

        // 连接系统托盘信号到窗口操作
        QObject::connect(&systemtray, &SystemTray::showMainWindowRequested, [window]() {
            window->show();
            window->raise();
            window->requestActivate();
        });

        QObject::connect(&systemtray, &SystemTray::showSettingsRequested, window, [window]() {
            // 通过 QML 打开设置窗口
            QMetaObject::invokeMethod(window, "openSettings");
        });

        QObject::connect(&systemtray, &SystemTray::changeWallpaperRequested, [&photoBackend]() {
            photoBackend.setRandomWallpaperFromLatest();
        });

        QObject::connect(&systemtray, &SystemTray::quitApplicationRequested, &app, &QGuiApplication::quit);
    }


    return app.exec();
}
