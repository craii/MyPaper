#include "PhotoBackend.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QDateTime>
#include <QString>
#include <QUrl>
#include <QProcess>
#include <QMessageBox>


#ifdef Q_OS_WIN
#include <windows.h>
#endif

//Application ID: 796849
//Access Key: 349bbp43dc8qwCj4953oHDAKAOzGdSxTUerHRy7LEkQ
//Secret key: nF4gkoJPfLlZbLFbsfrcV2gTf7Tx39NxucqE3TUMu60



// ========== PhotoModel 实现 ==========

PhotoModel::PhotoModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PhotoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_photos.count();
}

QVariant PhotoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_photos.count())
        return QVariant();

    const Photo &photo = m_photos.at(index.row());

    switch (role) {
    case ImageUrlRole:
        return photo.imageUrl;
    case TitleRole:
        return photo.title;
    case LikesRole:
        return photo.likes;
    case ViewsRole:
        return photo.views;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PhotoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageUrlRole] = "imageUrl";
    roles[TitleRole] = "title";
    roles[LikesRole] = "likes";
    roles[ViewsRole] = "views";
    return roles;
}

void PhotoModel::addPhoto(const QString &imageUrl, const QString &title, int likes, int views)
{
    beginInsertRows(QModelIndex(), m_photos.count(), m_photos.count());
    Photo photo;
    photo.imageUrl = imageUrl;
    photo.title = title;
    photo.likes = likes;
    photo.views = views;
    m_photos.append(photo);
    endInsertRows();
}

void PhotoModel::clearPhotos()
{
    beginResetModel();
    m_photos.clear();
    endResetModel();
}

void PhotoModel::loadSampleData()
{
    // 保留此方法以兼容旧接口，但不再使用
    qDebug() << "loadSampleData() is deprecated, use loadPhotosFromDirectory() instead";
}

// ========== PhotoBackend 实现 ==========

PhotoBackend::PhotoBackend(QObject *parent)
    : QObject(parent)
    , m_photoModel(new PhotoModel(this))
    , m_currentTabIndex(1) // 默认选中"最热"
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_autoWallpaperTimer(new QTimer(this))  // 添加定时器
    , m_currentStartIndex(0)
{
    // 连接文件系统监视器信号
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &PhotoBackend::onDirectoryChanged);

    // 连接自动更换壁纸定时器信号
    connect(m_autoWallpaperTimer, &QTimer::timeout,
            this, &PhotoBackend::onAutoWallpaperTimeout);


// 设置默认照片目录（可以根据需要修改）
#ifdef Q_OS_WIN
    QString defaultPath = QDir::homePath() + "/Pictures";
#else
    QString defaultPath = QDir::homePath() + "/Pictures";
#endif

    // 尝试加载默认目录
    if (QDir(defaultPath).exists()) {
        setPhotoDirectory(defaultPath);
    } else {
        qWarning() << "Default photo directory does not exist:" << defaultPath;
        qDebug() << "Please set photo directory using setPhotoDirectory()";
    }
    //壁纸服务器相关
    m_networkManager = new QNetworkAccessManager(this);
    m_cacheDirectory = m_photoDirectory_base + "/cache";
    ensureCacheDirectory();
}

PhotoBackend::~PhotoBackend()
{
}

void PhotoBackend::setPhotoDirectory(const QString &path)
{
    if (m_photoDirectory == path)
        return;

    QDir dir(path);
    if (!dir.exists()) {
        QString error = QString("Directory does not exist: %1").arg(path);
        qWarning() << error;
        emit errorOccurred(error);
        return;
    }

    // 移除旧的监视路径
    if (!m_photoDirectory.isEmpty()) {
        m_fileWatcher->removePath(m_photoDirectory);
    }

    m_photoDirectory = path;

    // 添加新的监视路径
    m_fileWatcher->addPath(m_photoDirectory);

    qDebug() << "Photo directory set to:" << m_photoDirectory;

    // 加载新目录中的照片
    loadPhotosFromDirectory(m_photoDirectory);

    emit photoDirectoryChanged();
}

void PhotoBackend::onPhotoClicked(int index)
{
    qDebug() << "Photo clicked at index:" << index;

    if (index >= 0 && index < m_photoModel->rowCount()) {
        QModelIndex modelIndex = m_photoModel->index(index, 0);
        QString imageUrl = m_photoModel->data(modelIndex, PhotoModel::ImageUrlRole).toString();
        QString title = m_photoModel->data(modelIndex, PhotoModel::TitleRole).toString();

        qDebug() << "Photo title:" << title;
        qDebug() << "Image URL:" << imageUrl;

        emit photoClicked(index, imageUrl);
    }
}

void PhotoBackend::refreshPhoto(int index)
{
    qDebug() << "Refreshing photo at index:" << index;

    // 重新扫描目录
    rescanDirectory();
}

// ===== 修改 switchTab 方法 =====
// 在 switchTab 方法中,修改最热(index=1)的处理逻辑:

void PhotoBackend::switchTab(int tabIndex)
{
    qDebug() << "Switching to tab:" << tabIndex;

    m_currentTabIndex = tabIndex;

    if (m_currentTabIndex == 0)
    {
        setPhotoDirectory(m_photoDirectory_base + "/latest");
        qDebug() << "读取文件夹latest: " << m_photoDirectory;
        refreshPhoto(m_currentTabIndex);
        sortPhotosByTab();
    }
    else if (m_currentTabIndex == 1)
    {
        // 检查是否配置了服务器
        if (!m_serverUrl.isEmpty() && !m_serverToken.isEmpty()) {
            qDebug() << "Server configured, loading from server";
            loadPhotosFromServer();
        } else {
            qDebug() << "No server config, loading from local hotest folder";
            setPhotoDirectory(m_photoDirectory_base + "/hotest");
            qDebug() << "读取文件夹hotest: " << m_photoDirectory;
            refreshPhoto(m_currentTabIndex);
            sortPhotosByTab();
        }
    }
    else if (m_currentTabIndex == 2)
    {
        setPhotoDirectory(m_photoDirectory_base + "/history");
        refreshPhoto(m_currentTabIndex);
        sortPhotosByTab();
    }
}

void PhotoBackend::loadMorePhotos()
{
    qDebug() << "Loading more photos...";

    if (m_allPhotoPaths.isEmpty()) {
        qDebug() << "No more photos to load";
        return;
    }

    // 如果已经加载完所有照片
    if (m_currentStartIndex >= m_allPhotoPaths.count()) {
        qDebug() << "All photos loaded";
        return;
    }

    emit photoLoadingStarted();

    // 计算下一批照片的范围
    int endIndex = qMin(m_currentStartIndex + PHOTOS_PER_PAGE, m_allPhotoPaths.count());

    qDebug() << "Loading photos from index" << m_currentStartIndex << "to" << (endIndex - 1);

    for (int i = m_currentStartIndex; i < endIndex; ++i) {
        QString filePath = m_allPhotoPaths.at(i);
        QString title = extractTitle(filePath);

        // 生成随机的点赞数和浏览量（模拟数据）
        int likes = QRandomGenerator::global()->bounded(100, 5000);
        int views = QRandomGenerator::global()->bounded(500, 30000);

        // 将file://前缀添加到路径以便QML Image组件加载
        QString imageUrl = QUrl::fromLocalFile(filePath).toString();

        m_photoModel->addPhoto(imageUrl, title, likes, views);
    }

    m_currentStartIndex = endIndex;

    qDebug() << "Total loaded:" << m_photoModel->rowCount() << "of" << m_allPhotoPaths.count();

    emit photoLoadingFinished();
}

void PhotoBackend::loadPhotosFromDirectory(const QString &directoryPath)
{
    QString path = directoryPath.isEmpty() ? m_photoDirectory : directoryPath;

    if (path.isEmpty()) {
        qWarning() << "No directory path specified";
        emit errorOccurred("No directory path specified");
        return;
    }

    qDebug() << "Loading photos from directory:" << path;

    emit photoLoadingStarted();

    // 清空现有数据
    m_photoModel->clearPhotos();
    m_allPhotoPaths.clear();
    m_currentStartIndex = 0;

    // 扫描目录
    scanDirectory(path);

    // 根据当前标签排序
    sortPhotosByTab();

    emit photoLoadingFinished();
}

void PhotoBackend::rescanDirectory()
{
    qDebug() << "Rescanning directory:" << m_photoDirectory;
    loadPhotosFromDirectory(m_photoDirectory);
}

void PhotoBackend::onDirectoryChanged(const QString &path)
{
    qDebug() << "Directory changed:" << path;

    // 自动重新加载照片
    rescanDirectory();
}

void PhotoBackend::scanDirectory(const QString &path)
{
    QDir dir(path);

    if (!dir.exists()) {
        QString error = QString("Directory does not exist: %1").arg(path);
        qWarning() << error;
        emit errorOccurred(error);
        return;
    }

    // 设置文件过滤器，只获取图片文件
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp"
                << "*.gif" << "*.svg" << "*.webp"
                << "*.JPG" << "*.JPEG" << "*.PNG" << "*.BMP";

    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    // 获取所有图片文件
    QFileInfoList fileList = dir.entryInfoList();

    qDebug() << "Found" << fileList.count() << "image files in directory";

    if (fileList.isEmpty()) {
        qWarning() << "No image files found in directory:" << path;
        emit errorOccurred("No image files found in directory");
        return;
    }

    // 将文件路径添加到列表
    for (const QFileInfo &fileInfo : fileList) {
        if (isSupportedImageFormat(fileInfo.absoluteFilePath())) {
            m_allPhotoPaths.append(fileInfo.absoluteFilePath());
        }
    }

    qDebug() << "Total supported images:" << m_allPhotoPaths.count();

    // 加载第一批照片
    loadMorePhotos();
}

void PhotoBackend::sortPhotosByTab()
{
    if (m_allPhotoPaths.isEmpty())
        return;

    emit photoLoadingStarted();

    // 清空当前显示
    m_photoModel->clearPhotos();
    m_currentStartIndex = 0;

    // 根据不同标签进行排序
    switch (m_currentTabIndex) {
    case 0: // 最新 - 按修改时间降序
        std::sort(m_allPhotoPaths.begin(), m_allPhotoPaths.end(),
                  [](const QString &a, const QString &b) {
                      QFileInfo fileA(a);
                      QFileInfo fileB(b);
                      return fileA.lastModified() > fileB.lastModified();
                  });
        qDebug() << "Sorted by: Most Recent";
        break;

    case 1: // 最热 - 按文件大小降序， 先这么地，日后再看有没有服务器存图片
        std::sort(m_allPhotoPaths.begin(), m_allPhotoPaths.end(),
                  [](const QString &a, const QString &b) {
                      QFileInfo fileA(a);
                      QFileInfo fileB(b);
                      return fileA.size() > fileB.size();
                  });
        qDebug() << "Sorted by: File Size (Hottest)";
        break;

    case 2: // 历史 - 按文件名字母顺序
        std::sort(m_allPhotoPaths.begin(), m_allPhotoPaths.end(),
                  [](const QString &a, const QString &b) {
                      QFileInfo fileA(a);
                      QFileInfo fileB(b);
                      return fileA.fileName() < fileB.fileName();
                  });
        qDebug() << "Sorted by: Name (Most Active)";
        break;
    }

    // 重新加载第一批照片
    loadMorePhotos();

    emit photoLoadingFinished();
}

bool PhotoBackend::isSupportedImageFormat(const QString &filePath) const
{
    QImageReader reader(filePath);
    return reader.canRead();
}

QString PhotoBackend::extractTitle(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    // 返回不带扩展名的文件名
    return fileInfo.baseName();
}


void PhotoBackend::set_application_dir(const QString &dir)
{

    this->application_dir =  dir;

}

QString PhotoBackend::get_application_dir()
{
    return this->application_dir;
}

void PhotoBackend::set_m_photoDirectory_base(const QString &dir)
{
    this->m_photoDirectory_base = dir;
}



void PhotoBackend::setWallPaper(const QString &path_to_image) const
{
    //qDebug() << "原始地址：" << path_to_image << Qt::endl;
    QString  local_path = QUrl(path_to_image).toLocalFile();//用QUrl处理特殊字符
    #ifdef Q_OS_WIN
        // Windows API 需要 wchar_t*,

        std::wstring path = local_path.toStdWString();

        qDebug() << "文件地址：" << path << Qt::endl;

        // 3. 设置桌面壁纸
        bool ok = SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            0,
            (PVOID)path.c_str(),
            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
            );

        if (!ok)
            qDebug() <<  "失败，设置壁纸失败！";
        else
            qDebug() <<  "成功，设置壁纸成功！";
    #endif

    #ifdef Q_OS_MACOS
        // ===== macOS =====
        QString script = QString(
                             "tell application \"System Events\"\n"
                             "set picture of every desktop to POSIX file \"%1\"\n"
                             "end tell"
                             ).arg(local_path);

        QStringList args;

        args << "-e" << script;

        int exitCode = QProcess::execute("osascript", args);

        qDebug() << "macOS osascript Exit Code:" << exitCode;


        if (exitCode != 0) {
            qDebug() << "macOS：设置壁纸失败";

            // 使用 Qt 显示权限提示对话框
            //这里不一定是权限问题，也有可能是图片路径不对，
            //但其他代码中似乎已经没有路径问题出现的可能，就默认是权限问题吧
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle("需要授权");
            msgBox.setText("应用需要自动化权限来设置壁纸");
            msgBox.setInformativeText(
                "请前往：\n"
                "系统偏好设置 > 安全性与隐私 > 隐私 > 自动化\n"
                "勾选允许 Wallpaper 控制 System Events"
                );
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();

        }
        else{
            qDebug() << "macOS：设置壁纸成功";
        }

    #endif

}



void PhotoBackend::startAutoWallpaperChange(int intervalMinutes)
{
    qDebug() << "Starting auto wallpaper change with interval:" << intervalMinutes << "minutes";

    if (intervalMinutes <= 0) {
        qWarning() << "Invalid interval, stopping auto wallpaper change";
        stopAutoWallpaperChange();
        return;
    }

    // 停止之前的定时器
    m_autoWallpaperTimer->stop();

    // 设置新的时间间隔（转换为毫秒）
    int intervalMs = intervalMinutes * 60 * 1000;
    m_autoWallpaperTimer->setInterval(intervalMs);

    // 启动定时器
    m_autoWallpaperTimer->start();

    qDebug() << "Auto wallpaper timer started with interval:" << intervalMs << "ms";
}

void PhotoBackend::stopAutoWallpaperChange()
{
    qDebug() << "Stopping auto wallpaper change";
    m_autoWallpaperTimer->stop();
}

void PhotoBackend::setRandomWallpaperFromLatest()
{
    qDebug() << "Setting random wallpaper from latest folder";

    // 构建 latest 文件夹路径
    QString latestPath = m_photoDirectory_base + "/latest";

    QDir dir(latestPath);
    if (!dir.exists()) {
        qWarning() << "Latest folder does not exist:" << latestPath;
        return;
    }

    // 设置文件过滤器，只获取图片文件
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp"
                << "*.gif" << "*.svg" << "*.webp"
                << "*.JPG" << "*.JPEG" << "*.PNG" << "*.BMP";

    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    // 获取所有图片文件
    QFileInfoList fileList = dir.entryInfoList();

    if (fileList.isEmpty()) {
        qWarning() << "No images found in latest folder:" << latestPath;
        return;
    }

    // 随机选择一张图片
    int randomIndex = QRandomGenerator::global()->bounded(fileList.count());
    QString randomImagePath = fileList.at(randomIndex).absoluteFilePath();

    qDebug() << "Selected random image:" << randomImagePath;

    // 设置为壁纸
    QString imageUrl = QUrl::fromLocalFile(randomImagePath).toString();
    setWallPaper(imageUrl);
}

void PhotoBackend::onAutoWallpaperTimeout()
{
    qDebug() << "Auto wallpaper timeout triggered";
    setRandomWallpaperFromLatest();
}

QString PhotoBackend::get_m_photoDirectory_base()
{
    return this->m_photoDirectory_base;
}

bool PhotoBackend::copyImage(const QString &sourcePath)
{
    QString  img_path = QUrl(sourcePath).toLocalFile();
    //qDebug()<< "img_path: -----> "<< img_path << "\n";
    QFileInfo file_info(img_path);
    QString destPath = m_photoDirectory_base + "/history/" + file_info.fileName();
    // 检查源文件是否存在
    if (!QFile::exists(img_path)) {
        qWarning() << "源文件不存在:" << img_path;
        return false;
    }

    // 确保目标目录存在，不存在则创建
    QFileInfo destInfo(destPath);
    QDir destDir = destInfo.absoluteDir();
    if (!destDir.exists()) {
        if (!destDir.mkpath(".")) {
            qWarning() << "无法创建目标目录:" << destDir.absolutePath();
            return false;
        }
    }

    // 如果目标文件已存在，先删除（允许覆盖）
    if (QFile::exists(destPath)) {
        if (!QFile::remove(destPath)) {
            qWarning() << "无法删除已存在的目标文件:" << destPath;
            return false;
        }
    }

    // 执行复制操作
    bool success = QFile::copy(img_path, destPath);

    if (!success) {
        qWarning() << "复制文件失败:" << img_path << "->" << destPath;
    } else {
        qDebug() << "文件复制成功:" << img_path << "->" << destPath;
    }

    return success;
}


// ===== 新增方法实现 =====

void PhotoBackend::setServerConfig(const QString &serverUrl, const QString &token)
{
    m_serverUrl = serverUrl.trimmed();
    m_serverToken = token.trimmed();

    qDebug() << "Server config updated - URL:" << m_serverUrl << "Token set:" << (!m_serverToken.isEmpty());
}

void PhotoBackend::ensureCacheDirectory()
{
    QDir cacheDir(m_cacheDirectory);
    if (!cacheDir.exists()) {
        if (cacheDir.mkpath(".")) {
            qDebug() << "Cache directory created:" << m_cacheDirectory;
        } else {
            qWarning() << "Failed to create cache directory:" << m_cacheDirectory;
        }
    }
}

void PhotoBackend::fetchWallpapersFromServer()
{
    if (m_serverUrl.isEmpty() || m_serverToken.isEmpty()) {
        qWarning() << "Server URL or token is empty, cannot fetch wallpapers";
        return;
    }

    qDebug() << "Fetching wallpapers from server:" << m_serverUrl;

    emit photoLoadingStarted();

    QUrl url(m_serverUrl + "/wallpaper");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString postData = QString("token=%1").arg(m_serverToken);

    QNetworkReply *reply = m_networkManager->post(request, postData.toUtf8());

    connect(reply, &QNetworkReply::finished, this, &PhotoBackend::onWallpapersFetched);
}

void PhotoBackend::onWallpapersFetched()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit photoLoadingFinished();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Network error: %1").arg(reply->errorString());
        qWarning() << error;
        emit serverError(error);
        emit photoLoadingFinished();
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "Server response:" << responseData;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject()) {
        QString error = "Invalid JSON response";
        qWarning() << error;
        emit serverError(error);
        emit photoLoadingFinished();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj["code"].toInt() != 200) {
        QString error = "Server returned error code: " + QString::number(jsonObj["code"].toInt());
        qWarning() << error;
        emit serverError(error);
        emit photoLoadingFinished();
        return;
    }

    // 获取 data_half 数组
    QJsonArray dataHalf = jsonObj["data_half"].toArray();

    if (dataHalf.isEmpty()) {
        qWarning() << "No images in data_half array";
        emit photoLoadingFinished();
        return;
    }

    // 清空现有数据
    m_photoModel->clearPhotos();
    m_serverImageUrls.clear();

    // 加载服务器图片列表
    for (const QJsonValue &value : dataHalf) {

        //添加token以便完成请求
        QString imageUrl = value.toString() + "?token=" + m_serverToken;
        m_serverImageUrls.append(imageUrl);

        // 提取图片名称作为标题
        QString title = extractImageNameFromUrl(imageUrl);

        // 生成随机的点赞数和浏览量(模拟数据)
        int likes = QRandomGenerator::global()->bounded(100, 5000);
        int views = QRandomGenerator::global()->bounded(500, 30000);

        // 添加到模型中
        //qDebug() <<"服务器图片：" << imageUrl << "\n";
        m_photoModel->addPhoto(imageUrl, title, likes, views);
    }

    qDebug() << "Loaded" << m_serverImageUrls.count() << "images from server";

    emit serverDataLoaded();
    emit photoLoadingFinished();
}

QString PhotoBackend::extractImageNameFromUrl(const QString &url)
{
    // 从 URL 中提取文件名
    // 例如: https://localhost/images/half_size_images/half_xxx.jpg -> half_xxx
    QUrl qUrl(url);
    QString path = qUrl.path();
    QFileInfo fileInfo(path);
    return fileInfo.baseName();
}

void PhotoBackend::downloadAndSetWallpaper(const QString &imageUrl)
{
    if (m_serverUrl.isEmpty() || m_serverToken.isEmpty()) {
        qWarning() << "Server URL or token is empty, cannot download image";
        return;
    }

    // 从 half_xxx.jpg 提取出中间的ID部分
    // 例如: half_3a552186e16e48388c49640769332185.jpg -> 3a552186e16e48388c49640769332185
    QString imageName = extractImageNameFromUrl(imageUrl);

    // 移除 "half_" 前缀
    if (imageName.startsWith("half_")) {
        imageName = imageName.mid(5); // 去掉前5个字符 "half_"
    }

    // 从原始URL中提取文件扩展名
    QFileInfo urlInfo(imageUrl);
    QString extension = urlInfo.suffix();
    if (extension.isEmpty()) {
        extension = "jpg"; // 默认扩展名
    }

    // 构建完整图片的URL 不用再加token，因为 onWallpapersFetched() 里面 已经加了 + "?token=" + m_serverToken 即 下方的 【extension】已经含token
    QString fullImageUrl = m_serverUrl + "/images/full_size_images/" + imageName + "." + extension ;

    qDebug() << "Downloading full image from:" << fullImageUrl;

    QNetworkRequest request{(QUrl(fullImageUrl))};
    QNetworkReply *reply = m_networkManager->get(request);

    m_currentDownloadUrl = fullImageUrl;

    connect(reply, &QNetworkReply::finished, this, &PhotoBackend::onImageDownloaded);
}

void PhotoBackend::onImageDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Image download error: %1").arg(reply->errorString());
        qWarning() << error;
        emit serverError(error);
        reply->deleteLater();
        return;
    }

    QByteArray imageData = reply->readAll();
    reply->deleteLater();

    if (imageData.isEmpty()) {
        qWarning() << "Downloaded image data is empty";
        return;
    }

    // 从URL中提取文件名
    QUrl url(m_currentDownloadUrl);
    QString path = url.path();
    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();

    // 移除token参数
    int queryIndex = fileName.indexOf('?');
    if (queryIndex != -1) {
        fileName = fileName.left(queryIndex);
    }

    // 保存到 hotest 文件夹
    QString savePath = m_photoDirectory_base + "/hotest/" + fileName;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();

        qDebug() << "Image saved to:" << savePath;

        // 设置为壁纸
        QString imageUrl = QUrl::fromLocalFile(savePath).toString();
        setWallPaper(imageUrl);

        // 复制到历史
        copyImage(imageUrl);

    } else {
        qWarning() << "Failed to save image to:" << savePath;
    }
}

void PhotoBackend::loadPhotosFromServer()
{
    qDebug() << "Loading photos from server mode";

    emit photoLoadingStarted();

    // 清空现有数据
    m_photoModel->clearPhotos();
    m_allPhotoPaths.clear();
    m_currentStartIndex = 0;

    // 从服务器获取图片列表
    fetchWallpapersFromServer();
}









//设置app默认尺寸
void PhotoBackend::SET_APP_WIDTH(int APP_WIDTH)
{
    this->app_width = APP_WIDTH;
}

void PhotoBackend::SET_APP_HEIGTH(int APP_HEIGTH)
{
     this->app_height = APP_HEIGTH;
}

//读取app尺寸
int PhotoBackend::get_APP_WIDTH()
{
    return this->app_width;
}

int PhotoBackend::get_APP_HEIGTH()
{
    return this->app_height;
}


//设置APP POSITION
void PhotoBackend::SET_APP_POSITION_X(int APP_POSITION_X)
{
    this->app_position_x = APP_POSITION_X;
}

void PhotoBackend::SET_APP_POSITION_Y(int APP_POSITION_Y)
{
    this->app_position_y = APP_POSITION_Y;
}

//读取APP POSITION
int PhotoBackend::get_APP_POSITION_X()
{
    return this->app_position_x;
}

int PhotoBackend::get_APP_POSITION_Y()
{
    return this->app_position_y;
}
