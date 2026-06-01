#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#pragma execution_character_set("utf-8")

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    static ConfigManager* instance();

    bool loadConfig(const QString& filePath);
    bool saveConfig(const QString& filePath);
    QJsonObject getConfig() const { return m_config; }
    void setConfig(const QJsonObject& config) { m_config = config; }

    // 获取特定模块的配置
    QJsonObject getModuleConfig(const QString& moduleName) const;
    void setModuleConfig(const QString& moduleName, const QJsonObject& moduleConfig);

signals:
    void configLoaded();
    void configSaved();

private:
    explicit ConfigManager(QObject *parent = nullptr);
    static ConfigManager* m_instance;
    QJsonObject m_config;
};

#endif // CONFIGMANAGER_H
