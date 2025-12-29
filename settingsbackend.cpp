#include "SettingsBackend.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>

SettingsBackend::SettingsBackend(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings("Pap.er", "Wallpaper", this))
{
    qDebug() << "SettingsBackend initialized";
}

SettingsBackend::~SettingsBackend()
{
}

QVariantMap SettingsBackend::loadSettings()
{
    QVariantMap settings;

    settings["wallpaperFolder"] = m_settings->value("wallpaperFolder", "").toString();
    settings["serverUrl"] = m_settings->value("server/url", "").toString();
    settings["token"] = m_settings->value("server/token", "").toString();
    settings["updateInterval"] = minutesToIntervalIndex(
        m_settings->value("updateInterval", 60).toInt()
        );

    qDebug() << "Settings loaded:" << settings;

    emit settingsLoaded(settings);
    return settings;
}

QVariantMap SettingsBackend::validateAndSave(
    const QString &wallpaperFolder,
    const QString &serverUrl,
    const QString &token,
    int updateInterval)
{
    QVariantMap result;
    result["success"] = false;

    QString folderError;
    QString serverError;

    // 验证壁纸文件夹
    bool folderValid = true;
    if (!wallpaperFolder.isEmpty()) {
        QString cleanedPath = cleanPath(wallpaperFolder);
        folderValid = validateFolderPath(cleanedPath, folderError);

        if (folderValid) {
            // 创建子文件夹
            if (!createSubfolders(cleanedPath, folderError)) {
                folderValid = false;
            }
        }
    }

    // 验证服务器配置（如果任何一个字段不为空）
    bool serverValid = true;
    if (!serverUrl.isEmpty() || !token.isEmpty()) {
        serverValid = validateServerConfig(serverUrl, token, serverError);
    }

    // 如果有错误，返回错误信息
    if (!folderValid || !serverValid) {
        if (!folderError.isEmpty()) {
            result["folderError"] = folderError;
        }
        if (!serverError.isEmpty()) {
            result["serverError"] = serverError;
        }
        return result;
    }

    // 保存设置
    if (!wallpaperFolder.isEmpty()) {
        m_settings->setValue("wallpaperFolder", cleanPath(wallpaperFolder));
    }

    m_settings->setValue("server/url", serverUrl);
    m_settings->setValue("server/token", token);
    m_settings->setValue("updateInterval", intervalIndexToMinutes(updateInterval));

    m_settings->sync();

    qDebug() << "Settings saved successfully";

    result["success"] = true;
    emit settingsSaved();

    return result;
}

QString SettingsBackend::getWallpaperFolder() const
{
    return m_settings->value("wallpaperFolder", "").toString();
}

QString SettingsBackend::getServerUrl() const
{
    return m_settings->value("server/url", "").toString();
}

QString SettingsBackend::getToken() const
{
    return m_settings->value("server/token", "").toString();
}

int SettingsBackend::getUpdateIntervalMinutes() const
{
    return m_settings->value("updateInterval", 60).toInt();
}

bool SettingsBackend::validateFolderPath(const QString &path, QString &errorMsg)
{
    if (path.isEmpty()) {
        errorMsg = "文件夹路径不能为空";
        return false;
    }

    // 检查路径是否过长
    if (path.length() > 260) {
        errorMsg = "路径过长（超过 260 字符）";
        return false;
    }

    QDir dir(path);

    // 如果目录不存在，尝试创建
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            errorMsg = "无法创建目录: " + path;
            return false;
        }
        qDebug() << "Created directory:" << path;
    }

    // 检查是否可写
    QFileInfo fileInfo(path);
    if (!fileInfo.isWritable()) {
        errorMsg = "目录不可写: " + path;
        return false;
    }

    return true;
}

bool SettingsBackend::createSubfolders(const QString &basePath, QString &errorMsg)
{
    QStringList subfolders = {"latest", "hotest", "history"};

    for (const QString &subfolder : subfolders) {
        QString subPath = basePath + "/" + subfolder;
        QDir dir(subPath);

        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                errorMsg = "无法创建子文件夹: " + subPath;
                return false;
            }
            qDebug() << "Created subfolder:" << subPath;
        }
    }

    return true;
}

bool SettingsBackend::validateServerConfig(
    const QString &serverUrl,
    const QString &token,
    QString &errorMsg)
{
    // 检查是否都已填写
    bool hasUrl = !serverUrl.isEmpty();
    bool hasToken = !token.isEmpty();

    // 如果有任何一个字段填写了，其他字段也必须填写
    if (hasUrl || hasToken) {
        if (!hasUrl) {
            errorMsg = "服务器地址不能为空";
            return false;
        }
        if (!hasToken) {
            errorMsg = "访问令牌不能为空";
            return false;
        }
    }

    // 验证服务器地址格式
    if (hasUrl) {
        // 检查是否以 http:// 或 https:// 开头
        if (!serverUrl.startsWith("http://") && !serverUrl.startsWith("https://")) {
            errorMsg = "服务器地址必须以 http:// 或 https:// 开头";
            return false;
        }

        // 检查长度
        if (serverUrl.length() < 10) {
            errorMsg = "服务器地址格式不正确（长度过短）";
            return false;
        }
    }

    // 验证 Token 格式
    if (hasToken) {
        if (token.length() < 10) {
            errorMsg = "访问令牌格式不正确（长度过短）";
            return false;
        }
    }

    return true;
}

QString SettingsBackend::cleanPath(const QString &path) const
{
    QString cleaned = path.trimmed();

    // 处理 ~ 符号（Unix/Linux 系统）
    if (cleaned.startsWith("~")) {
        cleaned.replace(0, 1, QDir::homePath());
    }

    // 标准化路径分隔符
    cleaned = QDir::cleanPath(cleaned);

    return cleaned;
}

int SettingsBackend::intervalIndexToMinutes(int index) const
{
    switch (index) {
    case 0: return 30;      // 30分钟
    case 1: return 60;      // 1小时
    case 2: return 1440;    // 1天
    case 3: return 0;
    default: return 0;     // 默认永不
    }
}

int SettingsBackend::minutesToIntervalIndex(int minutes) const
{
    if (minutes <= 30) return 0;
    if (minutes <= 60) return 1;
    return 2;
}
