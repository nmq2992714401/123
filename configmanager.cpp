#include "ConfigManager.h"
#include "Logger.h"

ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
}

ConfigManager* ConfigManager::instance()
{
    if (!m_instance)
        m_instance = new ConfigManager();
    return m_instance;
}

bool ConfigManager::loadConfig(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("无法打开配置文件: " + filePath);
        return false;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        LOG_ERROR("配置文件格式错误: " + filePath);
        return false;
    }
    m_config = doc.object();
    LOG_INFO("配置文件加载成功: " + filePath);
    emit configLoaded();
    return true;
}

bool ConfigManager::saveConfig(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("无法保存配置文件: " + filePath);
        return false;
    }
    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    file.close();
    LOG_INFO("配置文件保存成功: " + filePath);
    emit configSaved();
    return true;
}

QJsonObject ConfigManager::getModuleConfig(const QString& moduleName) const
{
    return m_config.value(moduleName).toObject();
}

void ConfigManager::setModuleConfig(const QString& moduleName, const QJsonObject& moduleConfig)
{
    m_config[moduleName] = moduleConfig;
}
