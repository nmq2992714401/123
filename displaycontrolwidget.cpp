#include "DisplayControlWidget.h"
//#include "BuoyParamDialog.h"
#include "GlobalDefines.h"
#include "logger.h"
#include "qgismanager.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
//#include <QDialog>
#include <cmath>

namespace
{
    const int kLocationTabIndex = 3;
    const int kBuoyRowCount = 12;
    const int kColumnSonarCoordinate = 1;
    const int kColumnCurrentTime = 2;
    const int kColumnMeasuredBearing = 3;
    const int kColumnDistance = 4;
    const int kColumnSnr = 5;
    const int kColumnConfidence = 6;
    const int kColumnTargetPosition = 7;
    const int kColumnConnectionStatus = 8;
    const int kColumnOperation = 9;
    const int kBuoyColumnCount = 5;

    // 浮标状态
    QString sonarBuoyStateToText(SonarBuoyState state)
    {
        switch (state) {
        case SonarBuoyState::PLANNED:
            return "未投放";
        case SonarBuoyState::FREE_FALL:
            return "自由落体";
        case SonarBuoyState::CHUTE_DESCENT:
            return "开伞减速";
        case SonarBuoyState::SPLASHED_WAITING:
            return "已落水，等待稳定";
        case SonarBuoyState::ACTIVE:
            return "工作中";
        default:
            return "未知状态";
        }
    }
    // 给浮标状态添加背景色以便于区分
    Qt::GlobalColor sonarBuoyStateColor(SonarBuoyState state)
    {
        switch (state) {
        case SonarBuoyState::PLANNED:
            return Qt::lightGray;
        case SonarBuoyState::FREE_FALL:
            return Qt::yellow;
        case SonarBuoyState::CHUTE_DESCENT:
            return Qt::cyan;
        case SonarBuoyState::SPLASHED_WAITING:
            return Qt::darkYellow;
        case SonarBuoyState::ACTIVE:
            return Qt::green;
        default:
            return Qt::white;
        }
    }

    QString displayNumberOrZero(double value, int precision = 4)
    {
        if (!std::isfinite(value)) {
            return QStringLiteral("0");
        }
        return QString::number(value, 'f', precision);
    }
}

DisplayControlWidget::DisplayControlWidget(QWidget *parent) : QWidget(parent)
{
    /*for (int i = 0; i < 12; ++i) {
        m_buoyLons.append(108.5 + ((double)qrand() / RAND_MAX));
        m_buoyLats.append(18.5 + ((double)qrand() / RAND_MAX));
    }*/
//    m_targetLon = 109.0;
//    m_targetLat = 18.8;
//    m_predictedLon = 109.02;
//    m_predictedLat = 18.82;

//    m_connectedBuoys = 12;
//    m_processDelay = 50;
//    m_recognitionRate = 90;

    m_locationError = 450;
    dist = 0.0;


    // 按照当前状态创建 5 个子页面和对应控件。
    setupUI();

    // 初始化12条列表数据
    for(int i = 0; i < 12; i++)
    {
        SonarStation det;
        det.id = i+1;
        det.latest_result.is_detected = true;
        det.latest_result.measured_bearing = 30 + i * 5.0 + (rand()%20) - 10.0;
        det.latest_result.true_bearing = 30 + i * 5.0;
        det.latest_result.bearing_error = det.latest_result.measured_bearing - det.latest_result.true_bearing;
        det.latest_result.snr = 15.0 + i * 2.0 + (rand()%100) / 10.0;
        det.latest_result.distance = 5000 + i * 500 + (rand()%2000);
        det.latest_result.target_position.latitude = 18.5 + i * 0.1 + (rand()%100) / 100.0;
        det.latest_result.target_position.longitude = 108.5 + i * 0.1 + (rand()%100) / 100.0;
        det.latest_result.target_position.vertical_position = 100;
        det.latest_result.confidence = 0.8 + i * 0.1 + (rand()%20) / 100.0;
        if(det.latest_result.confidence < 0) det.latest_result.confidence = 0;
        if(det.latest_result.confidence > 1) det.latest_result.confidence = 1;

        addDetectionResult(det.id, det.latest_result);                // 添加声纳检测结果
    }

    // 初始化经纬度
//    MotionState mon;
//    mon.position.latitude = 11.11;
//    mon.position.longitude = 12.12;
//    addMotionState(mon);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DisplayControlWidget::SimulationStep);
    m_timer->start(2000);

    m_mapFlushTimer = new QTimer(this);
    m_mapFlushTimer->setInterval(80);   // 12.5Hz，地图看起来已足够平滑
//    connect(m_mapFlushTimer, &QTimer::timeout,
//            this, &DisplayControlWidget::flushPendingMapUpdates);
//    m_mapFlushTimer->start();
}

void DisplayControlWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QTabWidget *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // ================== 页面2: 浮标状态 ==============================================
    QWidget* buoyPage = new QWidget(this);
    QVBoxLayout* buoyLayout = new QVBoxLayout(buoyPage);
    QGroupBox* buoyGroup = new QGroupBox("浮标状态", this);
    QVBoxLayout* buoyInner = new QVBoxLayout(buoyGroup);

    buoyTable = new QTableWidget(12, kBuoyColumnCount, this);
    buoyTable->setHorizontalHeaderLabels({"浮标ID", "经度", "纬度", "浮标状态", "高度(m)"});
    buoyTable->setMinimumHeight(300);
    for (int i = 0; i < kBuoyRowCount; ++i)
    {
        SonarStation son;
        son.id = i + 1;
        addSonarState(son);
    }
    buoyInner->addWidget(buoyTable);
    buoyLayout->addWidget(buoyGroup);
    buoyLayout->addStretch();
    tabWidget->addTab(buoyPage, "浮标状态");

    // =========================== 页面3: 目标识别 ==========================================
    QWidget* targetPage = new QWidget(this);
    QVBoxLayout* targetLayout = new QVBoxLayout(targetPage);

    // 潜艇目标真实位置
    QGroupBox* realTargetGroup = new QGroupBox("潜艇目标真实位置", this);
    QGridLayout *realLayout = new QGridLayout(realTargetGroup);
    realLayout->addWidget(new QLabel("经纬度: "), 0, 0);
    m_realPosLabel = new QLabel("--", this);
    realLayout->addWidget(m_realPosLabel, 0, 1);
    targetLayout->addWidget(realTargetGroup);

    // 平均置信度
    QGroupBox* targetGroup = new QGroupBox("平均置信度", this);
    QHBoxLayout* targetInner = new QHBoxLayout(targetGroup);
    targetType = new QLabel("潜艇", this);
    targetType->setStyleSheet("background-color: green; color: white; font-size: 24px; padding: 15px; border-radius: 5px;");
    targetConf = new QLabel("平均置信度: ", this);
    targetConf->setStyleSheet("font-size: 18px;");
    targetInner->addWidget(targetType);
    targetInner->addWidget(targetConf);
    targetLayout->addWidget(targetGroup);

    // 声纳检测结果
    QGroupBox *historyGroup = new QGroupBox("声纳检测结果", this);
    ceshiBtn = new QPushButton("测试发数据");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    m_detectionTable = new QTableWidget(this);
    m_detectionTable->setColumnCount(8);
    m_detectionTable->setHorizontalHeaderLabels({"检测结果ID", "时间", "方位角(°)", "距离(m)", "信噪比(dB)", "置信度", "经度","纬度"});
    m_detectionTable->horizontalHeader()->setStretchLastSection(true);
    m_detectionTable->setMinimumHeight(300);
    historyLayout->addWidget(m_detectionTable, 1);
    historyLayout->addWidget(ceshiBtn, 0);
    targetLayout->addWidget(historyGroup);

    targetLayout->addStretch();
    tabWidget->addTab(targetPage, "目标识别");

    // =============================== 页面4：主动声纳 =========================================
    QWidget *activeSonarPage = new QWidget(this);
    QVBoxLayout *activeSonarLayout = new QVBoxLayout(activeSonarPage);
    QGroupBox *activeSonarGroup = new QGroupBox("主动声纳检测结果", this);
    QVBoxLayout *activeSonarGroupLayout = new QVBoxLayout(activeSonarGroup);

    // 12个声纳固定行；列与 ActiveSonarContact 字段一一对应
    m_activeSonarTable = new QTableWidget(12, 23, this);
    m_activeSonarTable->setHorizontalHeaderLabels({
        "发射站ID",                 // emitter_station_id
        "ping发射",                 // ping_emitted-------
        "目标被照射",               // target_illuminated
        "检测到回波",               // echo_detected
        "ping时刻(s)",              // ping_time_s
        "真实距离(m)",              // true_range_m------------
        "估计距离(m)",              // estimated_range_m
        "真实方位(°)",              // true_bearing_deg----------
        "测得方位(°)",              // measured_bearing_deg
        "估计速度(kn)",             // estimated_speed_knots
        "传播损失(dB)",             // transmission_loss_db
        "目标强度(dB)",             // target_strength_db
        "入射级(dB)",               // incident_level_db
        "回波级(dB)",               // echo_level_db
        "信号余量(dB)",             // signal_excess_db
        "检测概率(%)",                 // detection_probability
        "接触置信度(%)",               // confidence
        "发射平台经度(°E)",         // emitter_position.longitude----
        "发射平台纬度(°N)",         // emitter_position.latitude-----
        "发射平台高度(m)",          // emitter_position.vertical_position------
        "目标估计经度(°E)",         // estimated_target_position.longitude
        "目标估计纬度(°N)",         // estimated_target_position.latitude
        "目标估计高度(m)"           // estimated_target_position.vertical_position
    });
    m_activeSonarTable->setMinimumHeight(700);
    m_activeSonarTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_activeSonarTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_activeSonarTable->horizontalHeader()->setStretchLastSection(true);

    // 初始化12行（声纳1~12），其余列先置 "--"
    for (int i = 0; i < 12; ++i) {
        m_activeSonarTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        for (int c = 1; c < 23; ++c) {
            m_activeSonarTable->setItem(i, c, new QTableWidgetItem("--"));
        }
    }

    activeSonarGroupLayout->addWidget(m_activeSonarTable);
    activeSonarLayout->addWidget(activeSonarGroup);
    activeSonarLayout->addStretch();

    tabWidget->addTab(activeSonarPage, "主动声纳");

    // =============================== 页面5: 定位结果 =========================================
    QWidget* locationPage = new QWidget(this);
    QVBoxLayout* locationLayout = new QVBoxLayout(locationPage);
    QGroupBox* mapGroup = new QGroupBox("定位结果", this);
    QVBoxLayout* mapLayout = new QVBoxLayout(mapGroup);

    m_qgisWidget = new QgisManager(this);

    QPixmap pixmap(":/img/plane.png");
    //-----
    m_qgisWidget->widget()->setMinimumHeight(700);
    m_qgisWidget->setGeoRange(108.0, 18.0, 110.0, 19.0);

    // 添加浮标
    for (int i = 0; i < kBuoyRowCount; ++i) {
        m_qgisWidget->addBuoy(i+1, m_buoyLons[i], m_buoyLats[i], true);
    }
//    m_qgisWidget->setSubmarinePosition(m_targetLon, m_targetLat);
    m_qgisWidget->setPredictedPosition(m_predictedLon, m_predictedLat, m_locationError);

    //-----
    mapLayout->addWidget(m_qgisWidget->widget());

    posLabel = new QLabel(QString("目标位置: %1°E, %2°N----误差: %3m")
                          .arg(m_targetLon)
                          .arg(m_targetLat)
                          .arg(m_locationError), this);
    mapLayout->addWidget(posLabel);

    locationLayout->addWidget(mapGroup);
    locationLayout->addStretch();
    tabWidget->addTab(locationPage, "定位结果");

}

// 添加声纳检测结果
void DisplayControlWidget::addDetectionResult(int id, const DetectionResult _det)
{
    if(!m_detectionTable) true;

    // 生成新行id
//    int newId = m_detectionTable->rowCount() + 1;

    int row = m_detectionTable->rowCount();
    m_detectionTable->insertRow(row);

    m_detectionTable->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
    // 获取当前时间
    QString timeStr = QDateTime::currentDateTime().toString("hh::mm::ss");
    m_detectionTable->setItem(row, 1, new QTableWidgetItem(timeStr));
    m_detectionTable->setItem(row, 2, new QTableWidgetItem(QString::number(_det.measured_bearing, 'f', 1)));
    m_detectionTable->setItem(row, 3, new QTableWidgetItem(QString::number(_det.distance, 'f', 0)));
    m_detectionTable->setItem(row, 4, new QTableWidgetItem(QString::number(_det.snr, 'f', 1)));
    m_detectionTable->setItem(row, 5, new QTableWidgetItem(QString::number(_det.confidence*100, 'f', 0) + "%"));
    QString pos = QString("%1 °E").arg(displayNumberOrZero(_det.target_position.longitude));
    m_detectionTable->setItem(row, 6, new QTableWidgetItem(pos));
    QString pos2 = QString("%1 °N").arg(displayNumberOrZero(_det.target_position.latitude));
    m_detectionTable->setItem(row, 7, new QTableWidgetItem(pos2));

    // 自动滚动到底部
    m_detectionTable->scrollToBottom();

//    LOG_INFO("显示控制模块：添加声纳检测结果");
}

// 声纳状态
void DisplayControlWidget::addSonarState(const SonarStation son)
{
    int stateValue = static_cast<int>(son.state);
    int row = son.id - 1;
    if (!buoyTable || row < 0 || row >= buoyTable->rowCount()) {
        return;
    }

    SonarBuoyState state = static_cast<SonarBuoyState>(stateValue);
    // 浮标id
    buoyTable->setItem(row, 0, new QTableWidgetItem(QString::number(son.id)));
    // 浮标经纬度
    QString lon = QString("%1 °E").arg(displayNumberOrZero(son.current_position.longitude));
    buoyTable->setItem(row, 1, new QTableWidgetItem(lon));
    QString lat = QString("%1 °N").arg(displayNumberOrZero(son.current_position.latitude));
    buoyTable->setItem(row, 2, new QTableWidgetItem(lat));
    // 浮标状态
    QTableWidgetItem *stateItem = new QTableWidgetItem(sonarBuoyStateToText(state));
    stateItem->setBackground(sonarBuoyStateColor(state));
    buoyTable->setItem(row, 3, stateItem);
    // 高度：海平面参考高度(m)，海面上为正，海面下为负
    QString height = QString("%1 m").arg(displayNumberOrZero(son.current_position.vertical_position));
    buoyTable->setItem(row, 4, new QTableWidgetItem(height));


    if (m_map_sonar_coord.contains(son.id)) {
        m_map_sonar_coord[son.id].state = state;
    }

}

// 更新浮标位置
void DisplayControlWidget::updateSonarPosition(const SonarStation son)
{
    //-----
    bool connected;
    // 查找指定浮标id
    m_map_sonar_coord[son.id] = son;

    //-----
    if(!m_qgisWidget->buoyExists[son.id]) {
        // 更新对应浮标的位置
        m_qgisWidget->updateBuoyPosition(son.id, son.current_position.longitude, son.current_position.latitude);
    } else {
        // 未找到指定id,添加新浮标
//        m_qgisWidget->addBuoy(son.id, son.current_position.longitude, son.current_position.latitude,connected);
        QMessageBox::information(this, "提示", "未找到指定id");
    }
}

// 添加目标运动状态
void DisplayControlWidget::addMotionState(const MotionState mon)
{
//    QString posText = QString("%1°E, %2°N")
//            .arg(displayNumberOrZero(mon.position.latitude))
//            .arg(displayNumberOrZero(mon.position.longitude));
//    m_realPosLabel->setText(posText);
//    m_cur_target_state = mon;

    // 先缓存数据
    m_cur_target_state = mon;

    // 仅当经纬度有效，认为“已收到真实潜艇数据”
    if (std::isfinite(mon.position.longitude) && std::isfinite(mon.position.latitude)) {
        m_hasLiveMotionState = true;
    }

    QString posText = QString("%1°E, %2°N")
            .arg(displayNumberOrZero(mon.position.latitude))
            .arg(displayNumberOrZero(mon.position.longitude));
    m_realPosLabel->setText(posText);
}

// 飞机状态
void DisplayControlWidget::updateAircraftState(const AircraftKinematics air)
{
    if (!m_qgisWidget) {
        return;
    }

    m_qgisWidget->setAircraftPosition(air.position.longitude, air.position.latitude, air.heading_deg, air.id);
}

// 鱼雷状态
void DisplayControlWidget::updateTorpedoState(const TorpedoStepOutput tor)
{
    if (!m_qgisWidget) {
        return;
    }

    // 获取当前鱼雷位置
    const GeoPosition &pos = tor.kinematics.position;
    if (std::isnan(pos.longitude) || std::isnan(pos.latitude)) return;

    // 鱼雷状态
    const int phase = static_cast<int>(tor.phase);
    const bool hitTarget = tor.kinematics.hit_target || phase == 3;

    m_qgisWidget->setTorpedoPosition(pos.longitude, pos.latitude, phase, hitTarget);
}

// 预测位置
void DisplayControlWidget::updateTrackingOutput(const TrackingOutput tra)
{
    if (!m_qgisWidget) {
        return;
    }

    // 把接收到的预测结果保存起来，否则 SimulationStep() 读到的还是旧值/默认值
    m_track = tra;

    // 收到预测位置后，立即刷新一次显示
    SimulationStep();
}

// 主动声纳
void DisplayControlWidget::updateActiveSonarContact(const ActiveSonarContact contact)
{
    if (!m_activeSonarTable) {
        return;
    }

    // 要求：只有 ping_emitted 为 true 才更新页面
    if (!contact.ping_emitted) {
        return;
    }

    // 声纳固定12个：ID范围 1~12
    const int sonarId = contact.emitter_station_id;
    if (sonarId < 1 || sonarId > 12) {
        return;
    }

    const int row = sonarId - 1;

    auto setCell = [this, row](int col, const QString &text) {
        QTableWidgetItem *item = m_activeSonarTable->item(row, col);
        if (!item) {
            item = new QTableWidgetItem;
            m_activeSonarTable->setItem(row, col, item);
        }
        item->setText(text);
    };

    // 与 ActiveSonarContact 字段严格对应
    setCell(0,  QString::number(contact.emitter_station_id));
    setCell(1,  contact.ping_emitted ? "true" : "false");
    setCell(2,  contact.target_illuminated ? "true" : "false");
    setCell(3,  contact.echo_detected ? "true" : "false");
    setCell(4,  displayNumberOrZero(contact.ping_time_s, 3));
    setCell(5,  displayNumberOrZero(contact.true_range_m, 2));
    setCell(6,  displayNumberOrZero(contact.estimated_range_m, 2));
    setCell(7,  displayNumberOrZero(contact.true_bearing_deg, 2));
    setCell(8,  displayNumberOrZero(contact.measured_bearing_deg, 2));
    setCell(9,  displayNumberOrZero(contact.estimated_speed_knots, 2));
    setCell(10, displayNumberOrZero(contact.transmission_loss_db, 2));
    setCell(11, displayNumberOrZero(contact.target_strength_db, 2));
    setCell(12, displayNumberOrZero(contact.incident_level_db, 2));
    setCell(13, displayNumberOrZero(contact.echo_level_db, 2));
    setCell(14, displayNumberOrZero(contact.signal_excess_db, 2));
    setCell(15, QString::number(contact.detection_probability * 100.0, 'f', 1) + "%");
    setCell(16, QString::number(contact.confidence * 100.0, 'f', 1) + "%");
    setCell(17, displayNumberOrZero(contact.emitter_position.longitude, 6));
    setCell(18, displayNumberOrZero(contact.emitter_position.latitude, 6));
    setCell(19, displayNumberOrZero(contact.emitter_position.vertical_position, 2));
    setCell(20, displayNumberOrZero(contact.estimated_target_position.longitude, 6));
    setCell(21, displayNumberOrZero(contact.estimated_target_position.latitude, 6));
    setCell(22, displayNumberOrZero(contact.estimated_target_position.vertical_position, 2));
}

// 测试按钮更新声纳检测结果
void DisplayControlWidget::updateDetectionResult(int id, const DetectionResult det)
{
    // 查找指定id的行
    int targetrow = -1;
    m_map_detciton_result[id] = det;
    for (int row = 0; row < m_detectionTable->rowCount(); ++row)
    {
        QTableWidgetItem *iditem = m_detectionTable->item(row, 0);
        if(iditem && iditem->text().toInt() == id)
        {
            targetrow = row;
            break;
        }
    }
    if(targetrow == -1)
    {
        // 未找到指定id,添加新行
        addDetectionResult(id, det);
        return;
    }

    // 更新数据
    QString timestr = QDateTime::currentDateTime().toString("hh::mm::ss");
    m_detectionTable->setItem(targetrow, 1, new QTableWidgetItem(timestr));
    m_detectionTable->setItem(targetrow, 2, new QTableWidgetItem(QString::number(det.measured_bearing, 'f', 1)));
    m_detectionTable->setItem(targetrow, 3, new QTableWidgetItem(QString::number(det.distance, 'f', 0)));
    m_detectionTable->setItem(targetrow, 4, new QTableWidgetItem(QString::number(det.snr, 'f', 1)));
    m_detectionTable->setItem(targetrow, 5, new QTableWidgetItem(QString::number(det.confidence * 100, 'f', 0) + "%"));
    QString pos = QString("%1 °e").arg(displayNumberOrZero(det.target_position.longitude));
    m_detectionTable->setItem(targetrow, 6, new QTableWidgetItem(pos));
    QString pos2 = QString("%1 °n").arg(displayNumberOrZero(det.target_position.latitude));
    m_detectionTable->setItem(targetrow, 7, new QTableWidgetItem(pos2));


    targetType->setText(det.is_detected ? "潜艇" : "非潜艇");
    // 更新平均置信度
    updateDetectionAvg();

    LOG_INFO("显示控制模块: 声纳检测结果已修改");
}

// 根据12条数据不同， 更新平均置信度
void DisplayControlWidget::updateDetectionAvg()
{
    double sum = 0.0;
    int count = 0;
    for(int i = 0; i < 12; i++)
    {
        QTableWidgetItem *item = m_detectionTable->item(i, 5);
        if(item)
        {
            QString text = item->text();
            if(text.endsWith("%"))
            {
                bool ok;
                double val = text.left(text.length() - 1).toDouble(&ok);
                if(ok)
                {
                    sum += val / 100.0;
                    count++;
                }
            }
        }
    }
    if(count > 0)
    {
        double avg = sum / count;
        targetConf->setText(QString("平均置信度: %1%").arg(avg * 100, 0, 'f', 1));
    }
}

// 目标定位算法(三角定位算法)
//GeoPosition DisplayControlWidget::calTargetPosition()
//{
//    double sum_lat = 0;
//    double sum_lon = 0;
//    int valid_intersections = 0;
//    int sonar_key = m_map_sonar_state.keys().size();
//    int detction_key = m_map_detciton_result.keys().size();
//    if (m_map_detciton_result.keys().size() < 2 || m_map_sonar_state.keys().size() < 2 || sonar_key != detction_key)
//    {
//        return GeoPosition();
//    }
//
//    // 遍历所有两两组合
//    for (size_t i = 0; i < sonar_key; ++i)
//    {
//        for(size_t j = i+1; j < sonar_key; ++j)
//        {
//            GeoPosition p = GeoUtils::calculateIntersection(
//                        m_map_sonar_state[i], m_map_detciton_result[i].measured_bearing,
//                        m_map_sonar_state[j], m_map_detciton_result[j].measured_bearing
//            );
//            if (p.isValid())
//            {
//                sum_lat += p.latitude;
//                sum_lon += p.longitude;
//            }
//
//        }
//    }
//
//    if(valid_intersections == 0) return GeoPosition();
//
//    // 计算几何中心
//    return GeoPosition(sum_lat / valid_intersections, sum_lon / valid_intersections, 0.0);
//}

// 仿真步进(目标位置更新)
void DisplayControlWidget::SimulationStep()
{
    GeoPosition target_pos = m_track.estimated_position;                // 预测目标位置定位
    GeoPosition target_pos_true = m_cur_target_state.position;          // 当前目标位置

    if (!std::isfinite(target_pos.vertical_position)) {
        target_pos.vertical_position = std::isfinite(target_pos_true.vertical_position)
                ? target_pos_true.vertical_position
                : 0.0;
    }

    if (!std::isfinite(target_pos_true.vertical_position)) {
        target_pos_true.vertical_position = target_pos.vertical_position;
    }

    const bool hasSubmarineData =
                m_hasLiveMotionState &&
                std::isfinite(target_pos_true.longitude) &&
                std::isfinite(target_pos_true.latitude);

//    dist = GeoUtils::calculateDistance(target_pos, target_pos_true);    // 误差
//    posLabel->setText(QString("预测位置: %1°E, %2°N--当前目标位置: %3°E, %4°N--误差: %5m")
//                      .arg(displayNumberOrZero(target_pos.longitude))
//                      .arg(displayNumberOrZero(target_pos.latitude))
//                      .arg(displayNumberOrZero(target_pos_true.longitude))
//                      .arg(displayNumberOrZero(target_pos_true.latitude))
//                      .arg(displayNumberOrZero(dist, 2)));
//    m_qgisWidget->setTargetPosition(target_pos_true.longitude, target_pos_true.latitude, dist);
//    m_qgisWidget->setPredictedPosition(target_pos.longitude, target_pos.latitude, dist);

    if (hasSubmarineData) {
        dist = GeoUtils::calculateDistance(target_pos, target_pos_true);

        posLabel->setText(QString("预测位置: %1°E, %2°N--当前目标位置: %3°E, %4°N--误差: %5m")
                          .arg(displayNumberOrZero(target_pos.longitude))
                          .arg(displayNumberOrZero(target_pos.latitude))
                          .arg(displayNumberOrZero(target_pos_true.longitude))
                          .arg(displayNumberOrZero(target_pos_true.latitude))
                          .arg(displayNumberOrZero(dist, 2)));

        // 关键：只有收到真实潜艇数据时，才显示潜艇图标/高亮
        m_qgisWidget->setTargetPosition(target_pos_true.longitude, target_pos_true.latitude, dist);
    } else {
        dist = 0.0;
        posLabel->setText(QString("预测位置: %1°E, %2°N--当前目标位置: -- --误差: --")
                          .arg(displayNumberOrZero(target_pos.longitude))
                          .arg(displayNumberOrZero(target_pos.latitude)));
    }

        // 预测点可以继续显示（按你现有逻辑保留）
//        m_qgisWidget->setPredictedPosition(target_pos.longitude, target_pos.latitude, dist);

}
