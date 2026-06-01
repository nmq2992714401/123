#include "BuoyParamDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

BuoyParamDialog::BuoyParamDialog(int buoyId, QWidget *parent)
    : QDialog(parent), m_buoyId(buoyId)
{
    // 对话框只服务于单个浮标，所以标题里直接带上浮标编号。
    setWindowTitle(QString("调整浮标 %1 参数").arg(buoyId));
    setModal(true);
    setFixedSize(350, 200);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 这里展示的是最常见的两个处理参数：FFT 点数和 LOFAR 时间窗。
    QFormLayout* formLayout = new QFormLayout();
    m_fftSpin = new QSpinBox(this);
    m_fftSpin->setRange(512, 4096);
    m_fftSpin->setValue(1024);
    m_fftSpin->setSingleStep(512);
    formLayout->addRow("FFT点数:", m_fftSpin);

    m_lofarSpin = new QSpinBox(this);
    m_lofarSpin->setRange(1, 10);
    m_lofarSpin->setValue(1);
    formLayout->addRow("LOFAR时间窗 (s):", m_lofarSpin);

    mainLayout->addLayout(formLayout);

    // 使用标准按钮盒，方便和 Qt 对话框交互习惯保持一致。
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &BuoyParamDialog::onSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void BuoyParamDialog::onSave()
{
    // 当前实现只做前端确认提示，实际参数下发逻辑可在这里继续扩展。
    accept();
    QMessageBox::information(this, "提示", QString("浮标 %1 参数已更新: FFT=%2, LOFAR=%3")
                             .arg(m_buoyId).arg(m_fftSpin->value()).arg(m_lofarSpin->value()));
}
