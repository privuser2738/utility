#include "sslconfig.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>
#include <QDateTime>
#include <QSslSocket>

SslConfig* SslConfig::m_instance = nullptr;

SslConfig* SslConfig::instance()
{
    if (!m_instance) {
        m_instance = new SslConfig();
    }
    return m_instance;
}

SslConfig::SslConfig(QObject* parent)
    : QObject(parent)
{
    loadOrGenerateCertificate();
}

QString SslConfig::configDir() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path;
}

QString SslConfig::certificatePath() const
{
    return configDir() + "/keycast.crt";
}

QString SslConfig::privateKeyPath() const
{
    return configDir() + "/keycast.key";
}

void SslConfig::loadOrGenerateCertificate()
{
    // Try to load existing certificate
    if (QFile::exists(certificatePath()) && QFile::exists(privateKeyPath())) {
        if (loadCertificate(certificatePath(), privateKeyPath())) {
            m_initialized = true;
            return;
        }
    }

    // Generate new self-signed certificate
    if (generateSelfSignedCertificate()) {
        m_initialized = true;
    }
}

bool SslConfig::hasCertificate() const
{
    return m_initialized && !m_certificate.isNull() && !m_privateKey.isNull();
}

bool SslConfig::generateSelfSignedCertificate()
{
    // Use OpenSSL to generate a self-signed certificate
    QString certPath = certificatePath();
    QString keyPath = privateKeyPath();

    // Create OpenSSL config for certificate generation
    QString configPath = configDir() + "/openssl.cnf";
    QFile configFile(configPath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        out << "[req]\n";
        out << "default_bits = 2048\n";
        out << "prompt = no\n";
        out << "default_md = sha256\n";
        out << "distinguished_name = dn\n";
        out << "x509_extensions = v3_req\n";
        out << "\n";
        out << "[dn]\n";
        out << "CN = KeyCast\n";
        out << "O = KeyCast\n";
        out << "\n";
        out << "[v3_req]\n";
        out << "subjectAltName = @alt_names\n";
        out << "basicConstraints = CA:FALSE\n";
        out << "keyUsage = digitalSignature, keyEncipherment\n";
        out << "extendedKeyUsage = serverAuth, clientAuth\n";
        out << "\n";
        out << "[alt_names]\n";
        out << "DNS.1 = localhost\n";
        out << "IP.1 = 127.0.0.1\n";
        configFile.close();
    }

    // Generate private key and certificate using OpenSSL
    QProcess process;
    QStringList args;
    args << "req" << "-x509" << "-newkey" << "rsa:2048"
         << "-keyout" << keyPath
         << "-out" << certPath
         << "-days" << "3650"
         << "-nodes"
         << "-config" << configPath;

    process.start("openssl", args);
    if (!process.waitForFinished(30000)) {
        emit error("Failed to generate certificate: OpenSSL timeout");
        return false;
    }

    if (process.exitCode() != 0) {
        // Try alternative method without config file
        args.clear();
        args << "req" << "-x509" << "-newkey" << "rsa:2048"
             << "-keyout" << keyPath
             << "-out" << certPath
             << "-days" << "3650"
             << "-nodes"
             << "-subj" << "/CN=KeyCast/O=KeyCast";

        process.start("openssl", args);
        if (!process.waitForFinished(30000) || process.exitCode() != 0) {
            emit error("Failed to generate certificate: " + process.readAllStandardError());
            return false;
        }
    }

    // Clean up config file
    QFile::remove(configPath);

    // Load the generated certificate
    return loadCertificate(certPath, keyPath);
}

bool SslConfig::loadCertificate(const QString& certPath, const QString& keyPath)
{
    // Load certificate
    QFile certFile(certPath);
    if (!certFile.open(QIODevice::ReadOnly)) {
        emit error("Failed to open certificate file: " + certPath);
        return false;
    }
    m_certificate = QSslCertificate(certFile.readAll(), QSsl::Pem);
    certFile.close();

    if (m_certificate.isNull()) {
        emit error("Failed to parse certificate");
        return false;
    }

    // Load private key
    QFile keyFile(keyPath);
    if (!keyFile.open(QIODevice::ReadOnly)) {
        emit error("Failed to open private key file: " + keyPath);
        return false;
    }
    m_privateKey = QSslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    keyFile.close();

    if (m_privateKey.isNull()) {
        emit error("Failed to parse private key");
        return false;
    }

    emit certificateChanged();
    return true;
}

bool SslConfig::saveCertificate(const QString& certPath, const QString& keyPath)
{
    if (!hasCertificate()) return false;

    QFile certFile(certPath);
    if (!certFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    certFile.write(m_certificate.toPem());
    certFile.close();

    QFile keyFile(keyPath);
    if (!keyFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    keyFile.write(m_privateKey.toPem());
    keyFile.close();

    return true;
}

QSslConfiguration SslConfig::serverConfiguration() const
{
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();

    if (hasCertificate()) {
        config.setLocalCertificate(m_certificate);
        config.setPrivateKey(m_privateKey);
    }

    config.setPeerVerifyMode(QSslSocket::VerifyNone); // Accept any client
    config.setProtocol(QSsl::TlsV1_2OrLater);

    return config;
}

QSslConfiguration SslConfig::clientConfiguration() const
{
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();

    config.setPeerVerifyMode(QSslSocket::VerifyNone); // Accept self-signed certs
    config.setProtocol(QSsl::TlsV1_2OrLater);

    return config;
}

QString SslConfig::certificateInfo() const
{
    if (!hasCertificate()) return "No certificate";

    QString info;
    info += "Subject: " + m_certificate.subjectInfo(QSslCertificate::CommonName).join(", ") + "\n";
    info += "Issuer: " + m_certificate.issuerInfo(QSslCertificate::CommonName).join(", ") + "\n";
    info += "Valid from: " + m_certificate.effectiveDate().toString() + "\n";
    info += "Valid until: " + m_certificate.expiryDate().toString();

    return info;
}

QDateTime SslConfig::certificateExpiry() const
{
    if (!hasCertificate()) return QDateTime();
    return m_certificate.expiryDate();
}
