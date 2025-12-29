#ifndef PHOTOBACKEND_H
#define PHOTOBACKEND_H

#include <QObject>
#include <QAbstractListModel>
#include <QString>
#include <QVariant>
#include <QFileSystemWatcher>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <QProcess>
#include <QTimer>

// 照片数据模型
class PhotoModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum PhotoRoles {
        ImageUrlRole = Qt::UserRole + 1,
        TitleRole,
        LikesRole,
        ViewsRole
    };

    explicit PhotoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 添加照片
    Q_INVOKABLE void addPhoto(const QString &imageUrl, const QString &title, int likes, int views);

    // 清空照片
    Q_INVOKABLE void clearPhotos();

    // 加载示例数据（已废弃，保留接口兼容性）
    Q_INVOKABLE void loadSampleData();

private:
    struct Photo {
        QString imageUrl;
        QString title;
        int likes;
        int views;
    };

    QList<Photo> m_photos;
};

// 照片后端业务逻辑
class PhotoBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PhotoModel* photoModel READ photoModel CONSTANT)
    Q_PROPERTY(QString photoDirectory READ photoDirectory WRITE setPhotoDirectory NOTIFY photoDirectoryChanged)

public:
    explicit PhotoBackend(QObject *parent = nullptr);
    ~PhotoBackend();

    PhotoModel* photoModel() const { return m_photoModel; }

    // 获取和设置照片目录
    QString photoDirectory() const { return m_photoDirectory; }
    void setPhotoDirectory(const QString &path);

    // 设置应用程序安装目录
    Q_INVOKABLE void set_application_dir(const QString &dir);
    Q_INVOKABLE QString get_application_dir();

    // 设置壁纸基础目录（可从QML调用）
    Q_INVOKABLE void set_m_photoDirectory_base(const QString &dir);
    Q_INVOKABLE QString get_m_photoDirectory_base();

    // 自动更换壁纸
    Q_INVOKABLE void startAutoWallpaperChange(int intervalMinutes);
    Q_INVOKABLE void stopAutoWallpaperChange();
    Q_INVOKABLE void setRandomWallpaperFromLatest();

    // 处理照片点击事件
    Q_INVOKABLE void onPhotoClicked(int index);

    // 刷新照片
    Q_INVOKABLE void refreshPhoto(int index);

    // 切换标签页
    Q_INVOKABLE void switchTab(int tabIndex);

    // 加载更多照片
    Q_INVOKABLE void loadMorePhotos();

    // 从本地目录加载所有图片
    Q_INVOKABLE void loadPhotosFromDirectory(const QString &directoryPath = QString());

    // 重新扫描当前目录
    Q_INVOKABLE void rescanDirectory();

    //设置壁纸
    Q_INVOKABLE void setWallPaper(const QString &path_to_image) const;

    //移动壁纸到历史
    Q_INVOKABLE bool copyImage(const QString &sourcePath);

    //设置app默认尺寸
    Q_INVOKABLE void SET_APP_WIDTH(int APP_WIDTH);
    Q_INVOKABLE void SET_APP_HEIGTH(int APP_HEIGTH);

    //读取app尺寸
    Q_INVOKABLE int get_APP_WIDTH();
    Q_INVOKABLE int get_APP_HEIGTH();

    //设置APP POSITION
    Q_INVOKABLE void SET_APP_POSITION_X(int APP_POSITION_X);
    Q_INVOKABLE void SET_APP_POSITION_Y(int APP_POSITION_Y);

    //读取APP POSITION
    Q_INVOKABLE  int get_APP_POSITION_X();
    Q_INVOKABLE int get_APP_POSITION_Y();

signals:
    // 通知 QML 更新
    void photoLoadingStarted();
    void photoLoadingFinished();
    void photoClicked(int index, const QString &imageUrl);
    void photoDirectoryChanged();
    void errorOccurred(const QString &errorMessage);

private slots:
    void onDirectoryChanged(const QString &path);
    void onAutoWallpaperTimeout();

private:
    PhotoModel *m_photoModel;

    // 当前选中的标签索引 (0=最新, 1=最热, 2=最活跃)
    int m_currentTabIndex;

    // 照片目录路径
    QString m_photoDirectory;
    QString m_photoDirectory_base;

    // 文件系统监视器
    QFileSystemWatcher *m_fileWatcher;

    // 自动更换壁纸定时器
    QTimer *m_autoWallpaperTimer;

    // 所有加载的图片路径列表
    QStringList m_allPhotoPaths;

    // 当前显示的起始索引
    int m_currentStartIndex;

    // 每次加载的照片数量
    static const int PHOTOS_PER_PAGE = 10;

    // 从目录加载图片文件
    void scanDirectory(const QString &path);

    // 根据标签排序照片
    void sortPhotosByTab();

    // 检查文件是否是支持的图片格式
    bool isSupportedImageFormat(const QString &filePath) const;

    // 从文件路径提取文件名作为标题
    QString extractTitle(const QString &filePath) const;

    //APP的安装位置
    QString application_dir;

    //APP SIZE
    int app_width;
    int app_height;

    //APP POSITION
    int app_position_x;
    int app_position_y;

};

#endif // PHOTOBACKEND_H
