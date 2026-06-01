#include "DataProcessingWidget.h"

#include <QDateTime>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QTextDocument>
#include <QRandomGenerator>

#include <cmath>

namespace {

const int kLocationTabIndex = 1;

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

DataProcessingWidget::DataProcessingWidget(QWidget* parent)
    : QWidget(parent)
    , m_totalCount(0)
    , m_parsedCount(0)
    , m_failedCount(0)
    , m_tabWidget(nullptr)
    , m_qgisWidget(nullptr)
    , m_processTable(nullptr)
    , m_detailView(nullptr)
    , m_posLabel(nullptr)
    , m_predictedLon(109.02)
    , m_predictedLat(18.82)
    , m_targetLon(109.0)
    , m_targetLat(18.8)
    , m_locationError(0.0)
{
//    initBuoyPositions();
    setupUI();
}

//void DataProcessingWidget::initBuoyPositions()
//{
//    m_buoyLons.clear();
//    m_buoyLats.clear();

//    for (int i = 0; i < 12; ++i) {
//        const double lon = 108.5 + QRandomGenerator::global()->bounded(1000) / 1000.0;
//        const double lat = 18.5 + QRandomGenerator::global()->bounded(500) / 1000.0;
//        m_buoyLons.append(lon);
//        m_buoyLats.append(lat);
//    }
//}

void DataProcessingWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_tabWidget = new QTabWidget(this);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DataProcessingWidget::onTabChanged);
    mainLayout->addWidget(m_tabWidget);

    // -----------------------------------------页面1：处理过程---------------------------------------------------------
    QWidget* processPage = new QWidget(this);
    QVBoxLayout* processLayout = new QVBoxLayout(processPage);
    processLayout->setContentsMargins(0, 0, 0, 0);
    processLayout->setSpacing(10);

    QGroupBox* summaryGroup = new QGroupBox("数据处理过程总览", this);
    QGridLayout* summaryLayout = new QGridLayout(summaryGroup);

    summaryLayout->addWidget(new QLabel("处理总条数:", this), 0, 0);
    m_totalCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_totalCountValue, 0, 1);

    summaryLayout->addWidget(new QLabel("可处理(解析成功):", this), 0, 2);
    m_parsedCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_parsedCountValue, 0, 3);

    summaryLayout->addWidget(new QLabel("不可处理(解析失败):", this), 0, 4);
    m_failedCountValue = new QLabel("0", this);
    summaryLayout->addWidget(m_failedCountValue, 0, 5);

    summaryLayout->addWidget(new QLabel("最近处理功能:", this), 1, 0);
    m_lastFunctionValue = new QLabel("-", this);
    summaryLayout->addWidget(m_lastFunctionValue, 1, 1, 1, 3);

    summaryLayout->addWidget(new QLabel("最近处理时间:", this), 1, 4);
    m_lastTimeValue = new QLabel("-", this);
    summaryLayout->addWidget(m_lastTimeValue, 1, 5);

    processLayout->addWidget(summaryGroup);

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);

    m_processTable = new QTableWidget(0, 5, this);
    m_processTable->setHorizontalHeaderLabels(QStringList()
                                              << "时间"
                                              << "功能"
                                              << "输入"
                                              << "处理过程"
                                              << "输出");
    m_processTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_processTable->horizontalHeader()->setStretchLastSection(true);
    m_processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_processTable->setAlternatingRowColors(true);
    splitter->addWidget(m_processTable);

    m_detailView = new QTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->document()->setMaximumBlockCount(500);
    splitter->addWidget(m_detailView);

    splitter->setSizes(QList<int>() << 420 << 220);
    processLayout->addWidget(splitter, 1);

    m_tabWidget->addTab(processPage, "处理过程");

    // -----------------------页面2：定位结果------------------------------------
    QWidget* locationPage = new QWidget(this);
    QVBoxLayout* locationLayout = new QVBoxLayout(locationPage);
    QGroupBox* mapGroup = new QGroupBox("定位结果", this);
    QVBoxLayout* mapLayout = new QVBoxLayout(mapGroup);

    m_qgisWidget = new QgisManager(this);
    m_qgisWidget->widget()->setMinimumHeight(600);
    m_qgisWidget->setGeoRange(108.0, 18.0, 110.0, 19.0);

    for (int i = 0; i < 12; ++i) {
        m_qgisWidget->addBuoy(i + 1, m_buoyLons[i], m_buoyLats[i], true);
    }
//    m_qgisWidget->setPredictedPosition(m_predictedLon, m_predictedLat, m_locationError);

    mapLayout->addWidget(m_qgisWidget->widget());

    m_posLabel = new QLabel(QString("目标位置: %1°E, %2°N----误差: %3m")
                            .arg(m_targetLon)
                            .arg(m_targetLat)
                            .arg(m_locationError), this);
    mapLayout->addWidget(m_posLabel);

    locationLayout->addWidget(mapGroup);
    locationLayout->addStretch();

    m_tabWidget->addTab(locationPage, "定位结果");
    m_tabWidget->setCurrentIndex(0);
}

void DataProcessingWidget::onTabChanged(int index)
{
    if (index == kLocationTabIndex && m_qgisWidget) {
        m_qgisWidget->setPredictedPosition(m_predictedLon, m_predictedLat, m_locationError);
    }
}

void DataProcessingWidget::updateCounters(bool parsed)
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

void DataProcessingWidget::appendProcessStep(const QString& functionName,
                                             const QString& input,
                                             const QString& process,
                                             const QString& output)
{
    const QString now = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_lastFunctionValue->setText(functionName);
    m_lastTimeValue->setText(now);

    m_processTable->insertRow(0);
    m_processTable->setItem(0, 0, new QTableWidgetItem(now));
    m_processTable->setItem(0, 1, new QTableWidgetItem(functionName));
    m_processTable->setItem(0, 2, new QTableWidgetItem(input));
    m_processTable->setItem(0, 3, new QTableWidgetItem(process));
    m_processTable->setItem(0, 4, new QTableWidgetItem(output));

    if (m_processTable->rowCount() > 500) {
        m_processTable->removeRow(500);
    }
}

void DataProcessingWidget::appendDetail(const QString& text)
{
    const QString now = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_detailView->append(QString("[%1] %2").arg(now, text));
}

QString DataProcessingWidget::formatDouble(double v, int precision) const
{
    if (!std::isfinite(v)) {
        return "--";
    }
    return QString::number(v, 'f', precision);
}

QString DataProcessingWidget::formatGeo(const GeoPosition& pos) const
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

void DataProcessingWidget::onRawDdsMessage(const QString& topic, int payloadSize, bool parsed)
{
    updateCounters(parsed);

    appendProcessStep(
        "DDS接收解析",
        QString("topic=%1, bytes=%2").arg(topic).arg(payloadSize),
        parsed ? "解包成功 -> Topic匹配 -> 路由业务处理" : "Topic不匹配或解包失败",
        parsed ? "进入业务处理链路" : "消息丢弃");

    appendDetail(QString("DDS消息 | topic=%1, bytes=%2, parsed=%3")
                 .arg(topic)
                 .arg(payloadSize)
                 .arg(parsed ? "true" : "false"));
}

void DataProcessingWidget::onDetectionResult(int id, const DetectionResult& det)
{
    appendProcessStep(
        "目标检测处理",
        QString("id=%1, bearing=%2, snr=%3, dist=%4")
        .arg(id)
        .arg(formatDouble(det.measured_bearing))
        .arg(formatDouble(det.snr))
        .arg(formatDouble(det.distance)),
        "特征提取 -> 检测判决 -> 置信度估计",
        QString("detected=%1, conf=%2, pos={%3}")
        .arg(det.is_detected ? "true" : "false")
        .arg(formatDouble(det.confidence, 3))
        .arg(formatGeo(det.target_position)));
}

void DataProcessingWidget::onSonarState(const SonarStation& son)
{
    appendProcessStep(
        "声纳状态融合",
        QString("id=%1, state=%2, pos={%3}")
        .arg(son.id)
        .arg(buoyStateToText(son.state))
        .arg(formatGeo(son.current_position)),
        "状态归一化 -> 态势更新 -> 结果融合",
        QString("latestDetected=%1, latestSNR=%2")
        .arg(son.latest_result.is_detected ? "true" : "false")
        .arg(formatDouble(son.latest_result.snr)));
}

void DataProcessingWidget::onMotionState(const MotionState& motion)
{
    appendProcessStep(
        "目标运动估计",
        QString("pos={%1}, speed=%2, heading=%3")
        .arg(formatGeo(motion.position))
        .arg(formatDouble(motion.speed))
        .arg(formatDouble(motion.heading)),
        "运动学更新 -> 滤波平滑",
        "目标运动状态已更新");
}

void DataProcessingWidget::onAircraftState(const AircraftKinematics& air)
{
    appendProcessStep(
        "平台状态处理",
        QString("id=%1, pos={%2}, speed=%3")
        .arg(air.id)
        .arg(formatGeo(air.position))
        .arg(formatDouble(air.speed_mps)),
        "平台参数更新 -> 任务约束刷新",
        QString("heading=%1, pitch=%2, roll=%3")
        .arg(formatDouble(air.heading_deg))
        .arg(formatDouble(air.pitch_deg))
        .arg(formatDouble(air.roll_deg)));
}

void DataProcessingWidget::onTorpedoState(const TorpedoStepOutput& tor)
{
    appendProcessStep(
        "鱼雷态势处理",
        QString("phase=%1, active=%2")
        .arg(torpedoPhaseToText(tor.phase))
        .arg(tor.kinematics.active ? "true" : "false"),
        "状态机推进 -> 命中/失效判定",
        QString("hit=%1, distToTarget=%2")
        .arg(tor.kinematics.hit_target ? "true" : "false")
        .arg(formatDouble(tor.kinematics.distance_to_target_m)));
}

void DataProcessingWidget::onTrackingOutput(const TrackingOutput& tra)
{
    appendProcessStep(
        "目标跟踪预测",
        QString("hasMeasurement=%1, hasTrack=%2")
        .arg(tra.has_measurement ? "true" : "false")
        .arg(tra.has_track ? "true" : "false"),
        "量测关联 -> 滤波更新 -> 轨迹外推",
        QString("estimate={%1}").arg(formatGeo(tra.estimated_position)));

    m_predictedLon = tra.estimated_position.longitude;
    m_predictedLat = tra.estimated_position.latitude;

    if (m_qgisWidget && m_tabWidget && m_tabWidget->currentIndex() == kLocationTabIndex) {
        m_qgisWidget->setPredictedPosition(m_predictedLon, m_predictedLat, m_locationError);
    }
}

void DataProcessingWidget::onActiveSonarContact(const ActiveSonarContact& act)
{
    appendProcessStep(
        "主动声纳处理",
        QString("station=%1, ping=%2, echo=%3")
        .arg(act.emitter_station_id)
        .arg(act.ping_emitted ? "true" : "false")
        .arg(act.echo_detected ? "true" : "false"),
        "回波检测 -> 测向测距 -> 位置反演",
        QString("range=%1, bearing=%2, conf=%3")
        .arg(formatDouble(act.estimated_range_m))
        .arg(formatDouble(act.measured_bearing_deg))
        .arg(formatDouble(act.confidence, 3)));
}
