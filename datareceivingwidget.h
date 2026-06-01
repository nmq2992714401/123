#ifndef DATARECEIVINGWIDGET_H
#define DATARECEIVINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>

#include "CommonTypes.h"
#include "ModelIO.h"

#pragma execution_character_set("utf-8")

class DataReceivingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataReceivingWidget(QWidget* parent = nullptr);

    // DDS 原始接收入口（每条消息都记）
    void onRawDdsMessage(const QString& topic, int payloadSize, bool parsed);

    // 业务类型详情（用于展示已解析后的关键字段）
    void onDetectionResult(int id, const DetectionResult& det);
    void onSonarState(const SonarStation& son);
    void onMotionState(const MotionState& motion);
    void onAircraftState(const AircraftKinematics& air);
    void onTorpedoState(const TorpedoStepOutput& tor);
    void onTrackingOutput(const TrackingOutput& tra);
    void onActiveSonarContact(const ActiveSonarContact& act);

private:
    void setupUI();
    void updateCounters(bool parsed);
    void appendRow(const QString& topic, int bytes, const QString& parseState,
                   const QString& bizType, const QString& summary);
    void appendDetail(const QString& text);
    QString formatDouble(double v, int precision = 2) const;
    QString formatGeo(const GeoPosition& pos) const;

private:
    QLabel* m_totalCountValue;
    QLabel* m_parsedCountValue;
    QLabel* m_failedCountValue;
    QLabel* m_lastTopicValue;
    QLabel* m_lastTimeValue;

    QTableWidget* m_table;
    QTextEdit* m_detailView;

    int m_totalCount;
    int m_parsedCount;
    int m_failedCount;
};

#endif // DATARECEIVINGWIDGET_H
