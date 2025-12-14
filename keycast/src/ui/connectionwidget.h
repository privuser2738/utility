#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget* parent = nullptr);
    ~ConnectionWidget() = default;

    void addClient(const QString& clientId, const QString& clientName, const QString& address);
    void removeClient(const QString& clientId);
    void clearClients();

    QString selectedClientId() const;
    QStringList allClientIds() const;

signals:
    void clientSelected(const QString& clientId);
    void disconnectRequested(const QString& clientId);

private:
    void setupUi();

    QListWidget* m_clientList;
    QLabel* m_countLabel;
    QPushButton* m_disconnectBtn;
};

#endif // CONNECTIONWIDGET_H
