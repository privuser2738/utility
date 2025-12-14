#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QDataStream>

namespace Protocol {

// Message types
enum class MessageType : quint8 {
    Auth = 0x01,
    AuthResponse = 0x02,
    KeyEvent = 0x10,
    MouseEvent = 0x11,
    MouseMove = 0x12,
    ExecuteCommand = 0x20,
    CommandOutput = 0x21,
    Ping = 0x30,
    Pong = 0x31,
    ClientInfo = 0x40,
    ServerInfo = 0x41,
    // Screen sharing
    ScreenShareRequest = 0x50,
    ScreenShareStart = 0x51,
    ScreenShareStop = 0x52,
    ScreenFrame = 0x53,
    ScreenFrameAck = 0x54,
    // Clipboard
    ClipboardData = 0x60,
    ClipboardRequest = 0x61,
    Disconnect = 0xFF
};

// Auth result codes
enum class AuthResult : quint8 {
    Success = 0x00,
    InvalidPassword = 0x01,
    ServerFull = 0x02,
    VersionMismatch = 0x03
};

// Protocol version
constexpr quint16 PROTOCOL_VERSION = 1;

// Magic header for discovery packets
constexpr quint32 DISCOVERY_MAGIC = 0x4B455943; // "KEYC"

// Packet structure helper
struct PacketHeader {
    quint32 magic = 0x4B435354; // "KCST"
    quint16 version = PROTOCOL_VERSION;
    MessageType type;
    quint32 payloadSize;
};

// Serialization functions
QByteArray createAuthPacket(const QString& password, const QString& clientName);
QByteArray createAuthResponsePacket(AuthResult result, const QString& serverName = QString());
QByteArray createKeyEventPacket(int vkCode, bool pressed);
QByteArray createMouseEventPacket(int x, int y, int button, bool pressed);
QByteArray createMouseMovePacket(int x, int y);
QByteArray createExecuteCommandPacket(const QString& command, const QString& type);
QByteArray createCommandOutputPacket(const QString& output);
QByteArray createPingPacket();
QByteArray createPongPacket();
QByteArray createClientInfoPacket(const QString& clientId, const QString& clientName);
QByteArray createDisconnectPacket();

// Screen sharing packets
QByteArray createScreenShareRequestPacket(bool start);
QByteArray createScreenFramePacket(const QByteArray& imageData, int width, int height, quint32 frameId);
QByteArray createScreenFrameAckPacket(quint32 frameId);

// Clipboard packets
QByteArray createClipboardDataPacket(const QString& mimeType, const QByteArray& data);
QByteArray createClipboardRequestPacket();

// Discovery packets
QByteArray createDiscoveryBroadcast(const QString& serverName, int port);
bool parseDiscoveryBroadcast(const QByteArray& data, QString& serverName, QString& address, int& port);

// Deserialization functions
bool parsePacketHeader(const QByteArray& data, PacketHeader& header);
bool parseAuthPacket(const QByteArray& data, QString& password, QString& clientName);
bool parseAuthResponsePacket(const QByteArray& data, AuthResult& result, QString& serverName);
bool parseKeyEventPacket(const QByteArray& data, int& vkCode, bool& pressed);
bool parseMouseEventPacket(const QByteArray& data, int& x, int& y, int& button, bool& pressed);
bool parseMouseMovePacket(const QByteArray& data, int& x, int& y);
bool parseExecuteCommandPacket(const QByteArray& data, QString& command, QString& type);
bool parseCommandOutputPacket(const QByteArray& data, QString& output);
bool parseClientInfoPacket(const QByteArray& data, QString& clientId, QString& clientName);

// Screen sharing parsing
bool parseScreenShareRequestPacket(const QByteArray& data, bool& start);
bool parseScreenFramePacket(const QByteArray& data, QByteArray& imageData, int& width, int& height, quint32& frameId);
bool parseScreenFrameAckPacket(const QByteArray& data, quint32& frameId);

// Clipboard parsing
bool parseClipboardDataPacket(const QByteArray& data, QString& mimeType, QByteArray& clipData);

} // namespace Protocol

#endif // PROTOCOL_H
