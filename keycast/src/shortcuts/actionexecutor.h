#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include <QObject>
#include <QProcess>
#include <QMap>

class ActionExecutor : public QObject
{
    Q_OBJECT

public:
    explicit ActionExecutor(QObject* parent = nullptr);
    ~ActionExecutor() = default;

public slots:
    void execute(const QString& command, const QString& type);
    void executeBinary(const QString& path, const QStringList& args = QStringList());
    void executeTerminal(const QString& command);

signals:
    void commandOutput(const QString& command, const QString& output);
    void commandError(const QString& command, const QString& error);
    void commandStarted(const QString& command);
    void commandFinished(const QString& command, int exitCode);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QString detectShell();
    QStringList shellArgs(const QString& command);

    QList<QProcess*> m_runningProcesses;
    QMap<QProcess*, QString> m_processCommands;
};

#endif // ACTIONEXECUTOR_H
