#ifndef SSLCONFIG_H
#define SSLCONFIG_H

#include <QObject>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QString>

class SslConfig : public QObject
{
    Q_OBJECT

public:
    static SslConfig* instance();

    // Get SSL configuration for server/client
    QSslConfiguration serverConfiguration() const;
    QSslConfiguration clientConfiguration() const;

    // Certificate management
    bool hasCertificate() const;
    bool generateSelfSignedCertificate();
    bool loadCertificate(const QString& certPath, const QString& keyPath);
    bool saveCertificate(const QString& certPath, const QString& keyPath);

    // Get paths
    QString certificatePath() const;
    QString privateKeyPath() const;

    // Certificate info
    QString certificateInfo() const;
    QDateTime certificateExpiry() const;

signals:
    void certificateChanged();
    void error(const QString& message);

private:
    explicit SslConfig(QObject* parent = nullptr);
    ~SslConfig() = default;

    void loadOrGenerateCertificate();
    QString configDir() const;

    static SslConfig* m_instance;

    QSslCertificate m_certificate;
    QSslKey m_privateKey;
    bool m_initialized = false;
};

#endif // SSLCONFIG_H
