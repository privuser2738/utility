#include "actionexecutor.h"
#include <QFileInfo>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ActionExecutor::ActionExecutor(QObject* parent)
    : QObject(parent)
{
}

void ActionExecutor::execute(const QString& command, const QString& type)
{
    if (type == "binary") {
        executeBinary(command);
    } else if (type == "terminal") {
        executeTerminal(command);
    }
}

void ActionExecutor::executeBinary(const QString& path, const QStringList& args)
{
    QString program = path;
    QStringList arguments = args;

    // Handle paths with arguments embedded
    if (args.isEmpty() && path.contains(' ')) {
        // Try to parse arguments from the path string
        QStringList parts;
        QString current;
        bool inQuotes = false;

        for (int i = 0; i < path.length(); ++i) {
            QChar c = path[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ' ' && !inQuotes) {
                if (!current.isEmpty()) {
                    parts.append(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.isEmpty()) {
            parts.append(current);
        }

        if (!parts.isEmpty()) {
            program = parts.takeFirst();
            arguments = parts;
        }
    }

    QProcess* process = new QProcess(this);
    process->setProgram(program);
    process->setArguments(arguments);
    process->setProcessChannelMode(QProcess::MergedChannels);

    // Set working directory to the binary's directory
    QFileInfo fi(program);
    if (fi.exists()) {
        process->setWorkingDirectory(fi.absolutePath());
    }

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ActionExecutor::onProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &ActionExecutor::onProcessError);
    connect(process, &QProcess::readyReadStandardOutput, this, &ActionExecutor::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &ActionExecutor::onReadyReadStandardError);

    m_runningProcesses.append(process);
    m_processCommands[process] = path;

    emit commandStarted(path);

    // Start detached for GUI applications, attached for console
    process->start();
}

void ActionExecutor::executeTerminal(const QString& command)
{
    QString shell = detectShell();
    QStringList args = shellArgs(command);

    QProcess* process = new QProcess(this);
    process->setProgram(shell);
    process->setArguments(args);
    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ActionExecutor::onProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &ActionExecutor::onProcessError);
    connect(process, &QProcess::readyReadStandardOutput, this, &ActionExecutor::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &ActionExecutor::onReadyReadStandardError);

    m_runningProcesses.append(process);
    m_processCommands[process] = command;

    emit commandStarted(command);
    process->start();
}

QString ActionExecutor::detectShell()
{
#ifdef Q_OS_WIN
    // Check for PowerShell first
    QString pwsh = qEnvironmentVariable("PROGRAMFILES") + "\\PowerShell\\7\\pwsh.exe";
    if (QFileInfo::exists(pwsh)) {
        return pwsh;
    }

    // Fall back to cmd.exe
    return "cmd.exe";
#else
    // Unix-like systems
    QString shell = qEnvironmentVariable("SHELL", "/bin/sh");
    return shell;
#endif
}

QStringList ActionExecutor::shellArgs(const QString& command)
{
#ifdef Q_OS_WIN
    QString shell = detectShell();
    if (shell.contains("pwsh") || shell.contains("powershell")) {
        return {"-NoProfile", "-Command", command};
    }
    // cmd.exe
    return {"/c", command};
#else
    return {"-c", command};
#endif
}

void ActionExecutor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString command = m_processCommands.value(process);

    // Read any remaining output
    QString output = QString::fromLocal8Bit(process->readAll());
    if (!output.isEmpty()) {
        emit commandOutput(command, output);
    }

    emit commandFinished(command, exitCode);

    m_runningProcesses.removeOne(process);
    m_processCommands.remove(process);
    process->deleteLater();
}

void ActionExecutor::onProcessError(QProcess::ProcessError error)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString command = m_processCommands.value(process);
    QString errorMsg;

    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "Failed to start process";
        break;
    case QProcess::Crashed:
        errorMsg = "Process crashed";
        break;
    case QProcess::Timedout:
        errorMsg = "Process timed out";
        break;
    case QProcess::WriteError:
        errorMsg = "Write error";
        break;
    case QProcess::ReadError:
        errorMsg = "Read error";
        break;
    default:
        errorMsg = "Unknown error";
    }

    emit commandError(command, errorMsg);
}

void ActionExecutor::onReadyReadStandardOutput()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString command = m_processCommands.value(process);
    QString output = QString::fromLocal8Bit(process->readAllStandardOutput());

    if (!output.isEmpty()) {
        emit commandOutput(command, output);
    }
}

void ActionExecutor::onReadyReadStandardError()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString command = m_processCommands.value(process);
    QString output = QString::fromLocal8Bit(process->readAllStandardError());

    if (!output.isEmpty()) {
        emit commandError(command, output);
    }
}
