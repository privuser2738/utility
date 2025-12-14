#include "protocol.h"
#include <QBuffer>

namespace Protocol {

static QByteArray createPacket(MessageType type, const QByteArray& payload)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Write header
    stream << static_cast<quint32>(0x4B435354); // Magic "KCST"
    stream << PROTOCOL_VERSION;
    stream << static_cast<quint8>(type);
    stream << static_cast<quint32>(payload.size());

    // Append payload
    packet.append(payload);

    return packet;
}

static QByteArray extractPayload(const QByteArray& packet)
{
    // Skip header (4 + 2 + 1 + 4 = 11 bytes)
    if (packet.size() < 11) return QByteArray();
    return packet.mid(11);
}

QByteArray createAuthPacket(const QString& password, const QString& clientName)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << password << clientName;
    return createPacket(MessageType::Auth, payload);
}

QByteArray createAuthResponsePacket(AuthResult result, const QString& serverName)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint8>(result) << serverName;
    return createPacket(MessageType::AuthResponse, payload);
}

QByteArray createKeyEventPacket(int vkCode, bool pressed)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<qint32>(vkCode) << pressed;
    return createPacket(MessageType::KeyEvent, payload);
}

QByteArray createMouseEventPacket(int x, int y, int button, bool pressed)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<qint32>(x) << static_cast<qint32>(y)
           << static_cast<qint32>(button) << pressed;
    return createPacket(MessageType::MouseEvent, payload);
}

QByteArray createMouseMovePacket(int x, int y)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<qint32>(x) << static_cast<qint32>(y);
    return createPacket(MessageType::MouseMove, payload);
}

QByteArray createExecuteCommandPacket(const QString& command, const QString& type)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << command << type;
    return createPacket(MessageType::ExecuteCommand, payload);
}

QByteArray createCommandOutputPacket(const QString& output)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << output;
    return createPacket(MessageType::CommandOutput, payload);
}

QByteArray createPingPacket()
{
    return createPacket(MessageType::Ping, QByteArray());
}

QByteArray createPongPacket()
{
    return createPacket(MessageType::Pong, QByteArray());
}

QByteArray createClientInfoPacket(const QString& clientId, const QString& clientName)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << clientId << clientName;
    return createPacket(MessageType::ClientInfo, payload);
}

QByteArray createDisconnectPacket()
{
    return createPacket(MessageType::Disconnect, QByteArray());
}

QByteArray createDiscoveryBroadcast(const QString& serverName, int port)
{
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << DISCOVERY_MAGIC << PROTOCOL_VERSION << serverName << static_cast<quint16>(port);
    return packet;
}

bool parseDiscoveryBroadcast(const QByteArray& data, QString& serverName, QString& address, int& port)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 magic;
    quint16 version;
    quint16 portValue;

    stream >> magic >> version >> serverName >> portValue;

    if (magic != DISCOVERY_MAGIC) return false;
    if (version != PROTOCOL_VERSION) return false;

    port = portValue;
    // address is set by the caller from the sender's IP
    return true;
}

bool parsePacketHeader(const QByteArray& data, PacketHeader& header)
{
    if (data.size() < 11) return false;

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::BigEndian);

    quint8 type;
    stream >> header.magic >> header.version >> type >> header.payloadSize;
    header.type = static_cast<MessageType>(type);

    return header.magic == 0x4B435354 && header.version == PROTOCOL_VERSION;
}

bool parseAuthPacket(const QByteArray& data, QString& password, QString& clientName)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> password >> clientName;
    return stream.status() == QDataStream::Ok;
}

bool parseAuthResponsePacket(const QByteArray& data, AuthResult& result, QString& serverName)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    quint8 r;
    stream >> r >> serverName;
    result = static_cast<AuthResult>(r);
    return stream.status() == QDataStream::Ok;
}

bool parseKeyEventPacket(const QByteArray& data, int& vkCode, bool& pressed)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    qint32 vk;
    stream >> vk >> pressed;
    vkCode = vk;
    return stream.status() == QDataStream::Ok;
}

bool parseMouseEventPacket(const QByteArray& data, int& x, int& y, int& button, bool& pressed)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    qint32 px, py, btn;
    stream >> px >> py >> btn >> pressed;
    x = px;
    y = py;
    button = btn;
    return stream.status() == QDataStream::Ok;
}

bool parseMouseMovePacket(const QByteArray& data, int& x, int& y)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    qint32 px, py;
    stream >> px >> py;
    x = px;
    y = py;
    return stream.status() == QDataStream::Ok;
}

bool parseExecuteCommandPacket(const QByteArray& data, QString& command, QString& type)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> command >> type;
    return stream.status() == QDataStream::Ok;
}

bool parseCommandOutputPacket(const QByteArray& data, QString& output)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> output;
    return stream.status() == QDataStream::Ok;
}

bool parseClientInfoPacket(const QByteArray& data, QString& clientId, QString& clientName)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> clientId >> clientName;
    return stream.status() == QDataStream::Ok;
}

// Screen sharing packets
QByteArray createScreenShareRequestPacket(bool start)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << start;
    return createPacket(start ? MessageType::ScreenShareStart : MessageType::ScreenShareStop, payload);
}

QByteArray createScreenFramePacket(const QByteArray& imageData, int width, int height, quint32 frameId)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << frameId << static_cast<qint32>(width) << static_cast<qint32>(height);
    stream << static_cast<quint32>(imageData.size());
    payload.append(imageData);
    return createPacket(MessageType::ScreenFrame, payload);
}

QByteArray createScreenFrameAckPacket(quint32 frameId)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << frameId;
    return createPacket(MessageType::ScreenFrameAck, payload);
}

QByteArray createClipboardDataPacket(const QString& mimeType, const QByteArray& data)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << mimeType << static_cast<quint32>(data.size());
    payload.append(data);
    return createPacket(MessageType::ClipboardData, payload);
}

QByteArray createClipboardRequestPacket()
{
    return createPacket(MessageType::ClipboardRequest, QByteArray());
}

bool parseScreenShareRequestPacket(const QByteArray& data, bool& start)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> start;
    return stream.status() == QDataStream::Ok;
}

bool parseScreenFramePacket(const QByteArray& data, QByteArray& imageData, int& width, int& height, quint32& frameId)
{
    QByteArray payload = extractPayload(data);
    if (payload.size() < 16) return false; // Minimum header size

    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    qint32 w, h;
    quint32 dataSize;
    stream >> frameId >> w >> h >> dataSize;

    width = w;
    height = h;

    // Extract image data (rest of payload after header)
    int headerSize = 4 + 4 + 4 + 4; // frameId + width + height + dataSize
    if (payload.size() < headerSize + static_cast<int>(dataSize)) return false;

    imageData = payload.mid(headerSize, dataSize);
    return true;
}

bool parseScreenFrameAckPacket(const QByteArray& data, quint32& frameId)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);
    stream >> frameId;
    return stream.status() == QDataStream::Ok;
}

bool parseClipboardDataPacket(const QByteArray& data, QString& mimeType, QByteArray& clipData)
{
    QByteArray payload = extractPayload(data);
    QDataStream stream(payload);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 dataSize;
    stream >> mimeType >> dataSize;

    // Calculate header size (variable due to string)
    int pos = payload.size() - dataSize;
    if (pos < 0) return false;

    clipData = payload.right(dataSize);
    return stream.status() == QDataStream::Ok;
}

} // namespace Protocol
