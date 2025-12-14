#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTabWidget>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() = default;

private slots:
    void onAccept();
    void onApply();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    QTabWidget* m_tabWidget;

    // General tab
    QCheckBox* m_startMinimizedCheck;
    QCheckBox* m_minimizeToTrayCheck;
    QCheckBox* m_closeToTrayCheck;
    QCheckBox* m_showNotificationsCheck;
    QLineEdit* m_computerNameEdit;

    // Network tab
    QSpinBox* m_serverPortSpin;
    QSpinBox* m_discoveryPortSpin;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_autoStartServerCheck;
    QCheckBox* m_enableDiscoveryCheck;

    // Mode tab
    QCheckBox* m_serverModeCheck;
    QCheckBox* m_clientModeCheck;

    // Broadcast tab
    QLineEdit* m_broadcastHotkeyEdit;
};

#endif // SETTINGSDIALOG_H
