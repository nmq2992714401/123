#ifndef DATAPROCESSINGWIDGET_H
#define DATAPROCESSINGWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QTabWidget>

#include "CommonTypes.h"
#include "ModelIO.h"
#include "qgismanager.h"

#pragma execution_character_set("utf-8")

class DataProcessingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataProcessingWidget(QWidget* parent = nullptr);

    void onRawDdsMessage(const QString& topic, int payloadSize, bool parsed);
    void onDetectionResult(int id, const DetectionResult& det);
    void onSonarState(const SonarStation& son);
    void onMotionState(const MotionState& motion);
    void onAircraftState(const AircraftKinematics& air);
    void onTorpedoState(const TorpedoStepOutput& tor);
    void onTrackingOutput(const TrackingOutput& tra);
    void onActiveSonarContact(const ActiveSonarContact& act);

private slots:
    void onTabChanged(int index);

private:
    void setupUI();
//    void initBuoyPositions();

    void updateCounters(bool parsed);
    void appendProcessStep(const QString& functionName,
                           const QString& input,
                           const QString& process,
                           const QString& output);
    void appendDetail(const QString& text);
    QString formatDouble(double v, int precision = 2) const;
    QString formatGeo(const GeoPosition& pos) const;

private:
    QLabel* m_totalCountValue;
    QLabel* m_parsedCountValue;
    QLabel* m_failedCountValue;
    QLabel* m_lastFunctionValue;
    QLabel* m_lastTimeValue;

    QTabWidget* m_tabWidget;
    QgisManager* m_qgisWidget;
    QTableWidget* m_processTable;
    QTextEdit* m_detailView;
    QLabel* m_posLabel;

    int m_totalCount;
    int m_parsedCount;
    int m_failedCount;

    QList<double> m_buoyLons;
    QList<double> m_buoyLats;

    double m_predictedLon;
    double m_predictedLat;
    double m_targetLon;
    double m_targetLat;
    double m_locationError;
};

#endif // DATAPROCESSINGWIDGET_H
