#ifndef BUOYPARAMDIALOG_H
#define BUOYPARAMDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

class BuoyParamDialog : public QDialog
{
    Q_OBJECT
public:
    // 浮标参数调整弹窗：从显示控制模块的浮标状态页进入。
    explicit BuoyParamDialog(int buoyId, QWidget *parent = nullptr);

    int getFftSize() const { return m_fftSpin->value(); }
    int getLofarWindow() const { return m_lofarSpin->value(); }

private slots:
    // 保存当前弹窗里的参数并关闭对话框。
    void onSave();

private:
    // 当前正在编辑的浮标编号。
    int m_buoyId;
    QSpinBox* m_fftSpin;
    QSpinBox* m_lofarSpin;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
};

#endif // BUOYPARAMDIALOG_H
