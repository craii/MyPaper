#ifndef SETTINGSBACKEND_H
#define SETTINGSBACKEND_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QSettings>

class SettingsBackend : public QObject
{
    Q_OBJECT

public:
    explicit SettingsBackend(QObject *parent = nullptr);
    ~SettingsBackend();

    // 加载设置
    Q_INVOKABLE QVariantMap loadSettings();

    // 验证并保存设置
    Q_INVOKABLE QVariantMap validateAndSave(
        const QString &wallpaperFolder,
        const QString &serverUrl,
        const QString &token,
        int updateInterval
        );

    // 获取当前壁纸文件夹
    Q_INVOKABLE QString getWallpaperFolder() const;

    // 获取服务器地址
    Q_INVOKABLE QString getServerUrl() const;

    // 获取 Token
    Q_INVOKABLE QString getToken() const;

    // 获取更新间隔（分钟）
    Q_INVOKABLE int getUpdateIntervalMinutes() const;

signals:
    // 设置加载完成
    void settingsLoaded(QVariantMap settings);

    // 设置保存成功
    void settingsSaved();

    // 错误信号
    void errorOccurred(const QString &error);

private:
    QSettings *m_settings;

    // 验证文件夹路径
    bool validateFolderPath(const QString &path, QString &errorMsg);

    // 创建必需的子文件夹
    bool createSubfolders(const QString &basePath, QString &errorMsg);

    // 验证服务器配置
    bool validateServerConfig(
        const QString &serverUrl,
        const QString &token,
        QString &errorMsg
        );

    // 清理路径（处理 ~ 等特殊符号）
    QString cleanPath(const QString &path) const;

    // 将索引转换为分钟数
    int intervalIndexToMinutes(int index) const;

    // 将分钟数转换为索引
    int minutesToIntervalIndex(int minutes) const;
};

#endif // SETTINGSBACKEND_H
