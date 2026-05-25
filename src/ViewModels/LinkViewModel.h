#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace gcs::comms { class LinkManager; }

namespace gcs::viewmodels {

// Thin QML adapter over LinkManager. Drives the ConnectionPanel: connection
// kind selection (UDP vs Serial), port/baud/host inputs, connect/disconnect
// buttons, and current status.
//
// Phase 7 adds Serial / Hardware Read-Only support. The Serial path is
// gated by SerialLink::isBackendAvailable() — if the Qt6::SerialPort module
// is not compiled in, attempts surface a clear error.
class LinkViewModel : public QObject
{
    Q_OBJECT

    // --- UDP (Phase 3) ---
    Q_PROPERTY(quint16 listenPort  READ listenPort  WRITE setListenPort NOTIFY configChanged)
    Q_PROPERTY(QString listenHost  READ listenHost  WRITE setListenHost NOTIFY configChanged)
    Q_PROPERTY(bool    connected   READ connected   NOTIFY stateChanged)
    Q_PROPERTY(bool    peerSeen    READ peerSeen    NOTIFY stateChanged)
    Q_PROPERTY(QString peerHint    READ peerHint    NOTIFY stateChanged)
    Q_PROPERTY(QString lastError   READ lastError   NOTIFY stateChanged)
    Q_PROPERTY(int     bytesIn     READ bytesIn     NOTIFY statsChanged)
    Q_PROPERTY(int     framesIn    READ framesIn    NOTIFY statsChanged)

    // --- Connection kind (Phase 7) ---
    // String form for easy QML ComboBox binding: "UDP" / "Serial".
    Q_PROPERTY(QString     connectionKind         READ connectionKind         WRITE setConnectionKind NOTIFY configChanged)
    Q_PROPERTY(QStringList connectionKindOptions  READ connectionKindOptions  CONSTANT)

    // --- Serial (Phase 7) ---
    Q_PROPERTY(bool        serialBackendAvailable READ serialBackendAvailable CONSTANT)
    Q_PROPERTY(QStringList availableSerialPorts   READ availableSerialPorts   NOTIFY serialPortsRefreshed)
    Q_PROPERTY(QString     serialPort             READ serialPort             WRITE setSerialPort NOTIFY configChanged)
    Q_PROPERTY(int         serialBaud             READ serialBaud             WRITE setSerialBaud NOTIFY configChanged)
    Q_PROPERTY(QStringList serialBaudOptions      READ serialBaudOptions      CONSTANT)
    Q_PROPERTY(bool        serialConnected        READ serialConnected        NOTIFY stateChanged)
    Q_PROPERTY(bool        hardwareReadOnlyActive READ hardwareReadOnlyActive NOTIFY stateChanged)

public:
    explicit LinkViewModel(gcs::comms::LinkManager* mgr, QObject* parent = nullptr);

    // UDP
    quint16 listenPort() const { return m_port; }
    QString listenHost() const { return m_host; }
    bool    connected()  const { return m_connected; }
    bool    peerSeen()   const { return m_peerSeen; }
    QString peerHint()   const { return m_peerHint; }
    QString lastError()  const { return m_lastError; }
    int     bytesIn()    const { return m_bytesIn; }
    int     framesIn()   const { return m_framesIn; }

    void setListenPort(quint16 p);
    void setListenHost(const QString& h);

    // Phase 7
    QString     connectionKind()         const { return m_connectionKind; }
    QStringList connectionKindOptions()  const { return {QStringLiteral("UDP"),
                                                          QStringLiteral("Serial")}; }
    void        setConnectionKind(const QString& kind);

    bool        serialBackendAvailable() const;
    QStringList availableSerialPorts()   const { return m_serialPorts; }
    QString     serialPort()             const { return m_serialPort; }
    int         serialBaud()             const { return m_serialBaud; }
    QStringList serialBaudOptions()      const;
    bool        serialConnected()        const { return m_serialConnected; }
    bool        hardwareReadOnlyActive() const { return m_serialConnected; }

    void setSerialPort(const QString& p);
    void setSerialBaud(int baud);

    Q_INVOKABLE bool connectToSitl();
    Q_INVOKABLE void disconnectFromSitl();

    Q_INVOKABLE void refreshSerialPorts();
    Q_INVOKABLE bool connectSerial();
    Q_INVOKABLE void disconnectSerial();

signals:
    void configChanged();
    void stateChanged();
    void statsChanged();
    void serialPortsRefreshed();
    void infoMessage(const QString& message);
    void errorMessage(const QString& message);

private slots:
    void refreshState();
    void onUdpError(const QString& msg);
    void onSerialError(const QString& msg);
    void onBytesIn(int n);
    void onUnknownMsg(int sysid, int compid, int msgid);

private:
    gcs::comms::LinkManager* m_mgr;
    quint16     m_port      = 14550;
    QString     m_host      = QStringLiteral("0.0.0.0");
    bool        m_connected = false;
    bool        m_peerSeen  = false;
    QString     m_peerHint;
    QString     m_lastError;
    int         m_bytesIn   = 0;
    int         m_framesIn  = 0;

    QString     m_connectionKind = QStringLiteral("UDP");
    QStringList m_serialPorts;
    QString     m_serialPort;
    int         m_serialBaud      = 115200;
    bool        m_serialConnected = false;
};

} // namespace gcs::viewmodels
