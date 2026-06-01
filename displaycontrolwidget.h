#ifndef DISPLAYCONTROLWIDGET_H
#define DISPLAYCONTROLWIDGET_H

#include <QDateTime>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QObject>
#include <QProgressBar>
#include <QSet>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>
#include <QPushButton>

#include "qgismanager.h"
//#include "structdef.h"
#include "ModelIO.h"

#pragma execution_character_set("utf-8")

class QTabWidget;
class QVBoxLayout;

namespace
{
    const int kColumnSonarId = 0;
}

class DisplayControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayControlWidget(QWidget *parent = nullptr);

    void addDetectionResult(int id, const DetectionResult _det);          // 添加声纳检测结果
    void addSonarState(const SonarStation son);                           // 添加声纳状态
    void addMotionState(const MotionState mon);                           // 添加目标运动状态
    QTableWidget *m_detectionTable;                                       // 显示控制模块的浮标页面的表格

    void updateDetectionResult(int id,const DetectionResult det);         // 更新列表数据
    void updateDetectionAvg();                                            // 更新置信度

    GeoPosition calTargetPosition();                               // 目标定位(三角定位算法)

    void SimulationStep();                                         // 仿真步进

    void updateSonarPosition(const SonarStation son);                // 更新浮标位置
    void updateAircraftState(const AircraftKinematics air);         // 更新飞机状态
    void updateTorpedoState(const TorpedoStepOutput tor);           // 更新鱼雷状态
    void updateTrackingOutput(const TrackingOutput tra);            // 预测位置
    void updateActiveSonarContact(const ActiveSonarContact act);    // 更新主动声纳页面

signals:
    void sig_calTargetPosition(GeoPosition);

public slots:
//    void onAdjustParamClicked(int buoyId);          // 调整参数按钮的槽函数

private slots:

private:
    void setupUI();

    // 页面1 实时监控
//    QLabel *m_connectedLabel;             // 显示当前连接浮标数。
//    QProgressBar *m_delayBar;             // 显示处理延迟。
//    QProgressBar *m_recognitionBar;       // 显示目标识别率。
//    QProgressBar *m_errorBar;             // 显示定位误差。
//    QTabWidget *m_tabWidget;

    // 页面4 定位结果
    QgisManager *m_qgisWidget = nullptr;
    QTimer *m_timer;                        // 仿真步进(目标位置更新)的时间延时

    QTimer *m_mapFlushTimer = nullptr;                 // 地图批量刷新定时器
//    QMap<int, SonarStation> m_pendingSonarMapUpdates; // 待刷新的浮标位置
//    AircraftState m_pendingAircraftState;             // 待刷新的飞机位置
//    bool m_hasPendingAircraftState = false;           // 是否有待刷新的飞机状态

    // 添加浮标的经纬度
    QList<double> m_buoyLons;
    QList<double> m_buoyLats;

    // 目标潜艇位置
    double m_targetLon;
    double m_targetLat;

    // 预测位置的经纬度和误差
    double m_predictedLon;
    double m_predictedLat;
    double m_locationError;

    QLabel *targetType;         // 潜艇
    QLabel *targetConf;         // 平均置信度

    QTableWidget *buoyTable;
    QLabel *m_realPosLabel;     // 目标真实位置
    QLabel *posLabel;           // 显示目标位置经纬度和误差

    QMap<int, DetectionResult> m_map_detciton_result;     // 根据声纳ID，存放检测结果
    QMap<int, SonarStation> m_map_sonar_coord;            // 根据声纳ID，存放声纳坐标
    //    QMap<int, GeoPosition> m_map_sonar_state;             // 根据声纳ID，存放声纳状态
//    QMap<int, int> m_buoyRowId;

    MotionState m_cur_target_state;       // 存放当前目标状态
    TrackingOutput m_track;               // 预测位置
    double dist;                          // 误差
//    DetectionResult m_det;              // 声纳状态



//    QVBoxLayout *m_locationMapLayout;
//    QLabel *m_mapPlaceholder;

    // 目标类型标签，例如“潜艇”。
    QLabel *m_targetTypeLabel;
    // 目标真实位置标签，显示在目标识别页最上方。
    QLabel *m_targetActualPosLabel;
    // 目标识别置信度标签。
    QLabel *m_targetConfLabel;
    // 地图下方的位置摘要文本。
    QLabel *m_posLabel;

    // ===== 主动声纳页 =====
    QTableWidget *m_activeSonarTable = nullptr;   // 主动声纳页面表格

    // ===== 当前显示状态缓存 =====
    // 这些值既是页面初始值，也是刷新时写回 UI 的数据来源。
    double m_averageConfidencePercent;
    bool m_hasLiveMotionState;
    bool m_hasLiveDetectionResult;
    bool m_moduleActive;
    bool m_uiRefreshPending;
    QSet<int> m_liveSonarIds;
    QMap<int, bool> m_sonarConnectedById;
    QMap<int, int> m_buoyRowById;
    QMap<int, double> m_latestConfidenceBySonar;
    QMap<int, DetectionResult> m_latestDetectionResultBySonar;
    QMap<int, QDateTime> m_latestDetectionTimeBySonar;

    // ===== 地图坐标缓存 =====
    // 浮标经纬度列表，用于初始化地图上的浮标点位。
    // 潜艇真实位置。
    double m_submarineLon;
    double m_submarineLat;


public:
    QPushButton *ceshiBtn;

};

#endif // DISPLAYCONTROLWIDGET_H
