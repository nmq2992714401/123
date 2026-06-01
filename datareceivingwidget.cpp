#include "DataReceivingWidget.h"

#include <QDateTime>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QTextDocument>

#include <cmath>

#pragma execution_character_set("utf-8")

namespace {

QString buoyStateToText(SonarBuoyState state)
{
    switch (state) {
    case SonarBuoyState::PLANNED: return "浮标未投放";
    case SonarBuoyState::FREE_FALL: return "浮标自由落体";
    case SonarBuoyState::CHUTE_DESCENT: return "浮标开伞减速";
    case SonarBuoyState::SPLASHED_WAITING: return "浮标已落水，等待稳定";
    case SonarBuoyState::ACTIVE: return "浮标工作中";
    default: return "浮标未知状态";
    }
}

QString torpedoPhaseToText(TorpedoPhase phase)
{
    switch (phase) {
    case TorpedoPhase::IDLE: return "鱼雷未发射";
    case TorpedoPhase::AIRBORNE: return "鱼雷空中下落";
    case TorpedoPhase::UNDERWATER: return "鱼雷入水后自主航行";
    case TorpedoPhase::HIT: return "鱼雷命中目标";
    case TorpedoPhase::EXPIRED: return "鱼雷超时、超程或其它失效";
    default: return "鱼雷未知状态";
    }
}

} // namespace

DataReceivingWidget::DataReceivingWidget(QWidget* parent)
    : QWidget(parent)
    , m_totalCount(0)
    , m_parsedCount(0)
    , m_failedCount(0)
{
    setupUI();
}

void DataReceivingWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QGroupBox* summaryGroup = new QGroupBox("DDS接收统计", this);
    QGridLayout* summaryLayout = new QGridLayout(summaryGroup);

    summaryLayout->addWidget(new QLabel("总接收条数:", this), 0, 0);
    m_totalCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_totalCountValue, 0, 1);

    summaryLayout->addWidget(new QLabel("解析成功:", this), 0, 2);
    m_parsedCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_parsedCountValue, 0, 3);

    summaryLayout->addWidget(new QLabel("未解析/失败:", this), 0, 4);
    m_failedCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_failedCountValue, 0, 5);

    summaryLayout->addWidget(new QLabel("最新Topic:", this), 1, 0);
    m_lastTopicValue = new QLabel("-", this);
    summaryLayout->addWidget(m_lastTopicValue, 1, 1, 1, 3);

    summaryLayout->addWidget(new QLabel("最新时间:", this), 1, 4);
    m_lastTimeValue = new QLabel("-", this);
    summaryLayout->addWidget(m_lastTimeValue, 1, 5);

    mainLayout->addWidget(summaryGroup);

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels(QStringList()
                                       << "时间"
                                       << "Topic"
                                       << "字节数"
                                       << "解析状态"
                                       << "业务类型"
                                       << "摘要");
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    splitter->addWidget(m_table);

    m_detailView = new QTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->document()->setMaximumBlockCount(300);
    splitter->addWidget(m_detailView);

    splitter->setSizes(QList<int>() << 420 << 220);
    mainLayout->addWidget(splitter, 1);
}

void DataReceivingWidget::updateCounters(bool parsed)
{
    ++m_totalCount;
    if (parsed) {
        ++m_parsedCount;
    } else {
        ++m_failedCount;
    }

    m_totalCountValue->setText(QString::number(m_totalCount));
    m_parsedCountValue->setText(QString::number(m_parsedCount));
    m_failedCountValue->setText(QString::number(m_failedCount));
}

void DataReceivingWidget::appendRow(const QString& topic, int bytes, const QString& parseState,
                                    const QString& bizType, const QString& summary)
{
    const QString now = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_lastTopicValue->setText(topic);
    m_lastTimeValue->setText(now);

    m_table->insertRow(0);
    m_table->setItem(0, 0, new QTableWidgetItem(now));
    m_table->setItem(0, 1, new QTableWidgetItem(topic));
    m_table->setItem(0, 2, new QTableWidgetItem(QString::number(bytes)));
    m_table->setItem(0, 3, new QTableWidgetItem(parseState));
    m_table->setItem(0, 4, new QTableWidgetItem(bizType));
    m_table->setItem(0, 5, new QTableWidgetItem(summary));

    if (m_table->rowCount() > 500) {
        m_table->removeRow(500);
    }
}

void DataReceivingWidget::appendDetail(const QString& text)
{
    const QString now = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_detailView->append(QString("[%1] %2").arg(now, text));
}

QString DataReceivingWidget::formatDouble(double v, int precision) const
{
    if (!std::isfinite(v)) {
        return "--";
    }
    return QString::number(v, 'f', precision);
}

QString DataReceivingWidget::formatGeo(const GeoPosition& pos) const
{
    if (!std::isfinite(pos.latitude) ||
        !std::isfinite(pos.longitude) ||
        !std::isfinite(pos.vertical_position)) {
        return "--";
    }

    return QString("lat=%1, lon=%2, z=%3")
            .arg(formatDouble(pos.latitude, 6))
            .arg(formatDouble(pos.longitude, 6))
            .arg(formatDouble(pos.vertical_position, 1));
}

void DataReceivingWidget::onRawDdsMessage(const QString& topic, int payloadSize, bool parsed)
{
    updateCounters(parsed);

    appendRow(topic,
              payloadSize,
              parsed ? "已解析" : "未解析",
              "DDS接收",
              parsed ? "已进入业务处理" : "未匹配业务Topic或解包失败");
}

void DataReceivingWidget::onDetectionResult(int id, const DetectionResult& det)
{
    appendDetail(QString("声纳检测结果 | id=%1, detected=%2, bearing=%3°, snr=%4dB, dist=%5m, conf=%6, pos={%7}")
                 .arg(id)
                 .arg(det.is_detected ? "true" : "false")
                 .arg(formatDouble(det.measured_bearing))
                 .arg(formatDouble(det.snr))
                 .arg(formatDouble(det.distance))
                 .arg(formatDouble(det.confidence, 3))
                 .arg(formatGeo(det.target_position)));
}

void DataReceivingWidget::onSonarState(const SonarStation& son)
{
    appendDetail(QString("SonarState | id=%1, state=%2, pos={%3}, latestDetected=%4")
                 .arg(son.id)
                 .arg(buoyStateToText(son.state))
                 .arg(formatGeo(son.current_position))
                 .arg(son.latest_result.is_detected ? "true" : "false"));
}

void DataReceivingWidget::onMotionState(const MotionState& motion)
{
    appendDetail(QString("目标运动状态 | pos={%1}, speed=%2kn, heading=%3°")
                 .arg(formatGeo(motion.position))
                 .arg(formatDouble(motion.speed))
                 .arg(formatDouble(motion.heading)));
}

void DataReceivingWidget::onAircraftState(const AircraftKinematics& air)
{
    appendDetail(QString("飞机状态 | id=%1, pos={%2}, speed=%3m/s, heading=%4°, pitch=%5°, roll=%6°")
                 .arg(air.id)
                 .arg(formatGeo(air.position))
                 .arg(formatDouble(air.speed_mps))
                 .arg(formatDouble(air.heading_deg))
                 .arg(formatDouble(air.pitch_deg))
                 .arg(formatDouble(air.roll_deg)));
}

void DataReceivingWidget::onTorpedoState(const TorpedoStepOutput& tor)
{
    appendDetail(QString("鱼雷状态 | phase=%1, active=%2, inWater=%3, hit=%4, distToTarget=%5m, pos={%6}")
                 .arg(torpedoPhaseToText(tor.phase))
                 .arg(tor.kinematics.active ? "true" : "false")
                 .arg(tor.kinematics.in_water ? "true" : "false")
                 .arg(tor.kinematics.hit_target ? "true" : "false")
                 .arg(formatDouble(tor.kinematics.distance_to_target_m))
                 .arg(formatGeo(tor.kinematics.position)));
}

void DataReceivingWidget::onTrackingOutput(const TrackingOutput& tra)
{
    appendDetail(QString("预测位置 | hasMeasurement=%1, hasTrack=%2, estimate={%3}")
                 .arg(tra.has_measurement ? "true" : "false")
                 .arg(tra.has_track ? "true" : "false")
                 .arg(formatGeo(tra.estimated_position)));
}

void DataReceivingWidget::onActiveSonarContact(const ActiveSonarContact& act)
{
    appendDetail(QString("主动声纳 | station=%1, ping=%2, echo=%3, range=%4m, bearing=%5°, conf=%6, estPos={%7}")
                 .arg(act.emitter_station_id)
                 .arg(act.ping_emitted ? "true" : "false")
                 .arg(act.echo_detected ? "true" : "false")
                 .arg(formatDouble(act.estimated_range_m))
                 .arg(formatDouble(act.measured_bearing_deg))
                 .arg(formatDouble(act.confidence, 3))
                 .arg(formatGeo(act.estimated_target_position)));
}
