#include "connectionwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void ConnectionWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_countLabel = new QLabel("Connected Clients: 0");
    layout->addWidget(m_countLabel);

    m_clientList = new QListWidget();
    m_clientList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_clientList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current) {
        if (current) {
            emit clientSelected(current->data(Qt::UserRole).toString());
        }
    });
    layout->addWidget(m_clientList);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    m_disconnectBtn = new QPushButton("Disconnect Selected");
    m_disconnectBtn->setEnabled(false);
    connect(m_disconnectBtn, &QPushButton::clicked, this, [this]() {
        QString clientId = selectedClientId();
        if (!clientId.isEmpty()) {
            emit disconnectRequested(clientId);
        }
    });
    connect(m_clientList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current) {
        m_disconnectBtn->setEnabled(current != nullptr);
    });
    btnLayout->addWidget(m_disconnectBtn);

    btnLayout->addStretch();
    layout->addLayout(btnLayout);
}

void ConnectionWidget::addClient(const QString& clientId, const QString& clientName, const QString& address)
{
    // Check if already exists
    for (int i = 0; i < m_clientList->count(); ++i) {
        if (m_clientList->item(i)->data(Qt::UserRole).toString() == clientId) {
            // Update existing
            m_clientList->item(i)->setText(QString("%1 (%2)").arg(clientName, address));
            return;
        }
    }

    QListWidgetItem* item = new QListWidgetItem(QString("%1 (%2)").arg(clientName, address));
    item->setData(Qt::UserRole, clientId);
    item->setData(Qt::UserRole + 1, clientName);
    item->setData(Qt::UserRole + 2, address);
    m_clientList->addItem(item);

    m_countLabel->setText(QString("Connected Clients: %1").arg(m_clientList->count()));
}

void ConnectionWidget::removeClient(const QString& clientId)
{
    for (int i = 0; i < m_clientList->count(); ++i) {
        if (m_clientList->item(i)->data(Qt::UserRole).toString() == clientId) {
            delete m_clientList->takeItem(i);
            break;
        }
    }

    m_countLabel->setText(QString("Connected Clients: %1").arg(m_clientList->count()));
}

void ConnectionWidget::clearClients()
{
    m_clientList->clear();
    m_countLabel->setText("Connected Clients: 0");
}

QString ConnectionWidget::selectedClientId() const
{
    QListWidgetItem* item = m_clientList->currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QStringList ConnectionWidget::allClientIds() const
{
    QStringList ids;
    for (int i = 0; i < m_clientList->count(); ++i) {
        ids.append(m_clientList->item(i)->data(Qt::UserRole).toString());
    }
    return ids;
}
