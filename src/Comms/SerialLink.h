#pragma once

#include <QList>
#include <QString>
#include <memory>

#include "LinkInterface.h"

class QSerialPort;

namespace gcs::comms {

// Read-only serial transport for connecting to a flight controller over
// USB or telemetry radio (HITL / real hardware bring-up).
//
// SAFETY (Phase 7):
//   - This build is telemetry-monitoring only. `writeBytes` is implemented
//     for the LinkInterface contract but explicitly REJECTS writes on a
//     Serial link in this phase (additional defence beyond "no caller
//     constructs MAVLink").
//   - `kind() == LinkKind::Serial` so SafetyGate can refuse manual control
//     and mission upload to a vehicle on this link, even when the autopilot
//     reports `PX4` / `ArduPilot` (real Pixhawk vs SITL are otherwise
//     indistinguishable via MAVLink).
//
// Two backends behind GCS_HAS_SERIAL_PORT:
//   - Defined  -> real QSerialPort I/O.
//   - Undefined-> graceful stub. `connectLink()` emits an `errorOccurred`
//                 explaining how to enable the module, and stays
//                 disconnected. The public API, name(), kind(), tests, and
//                 LinkManager integration are identical, so installing the
//                 Qt6::SerialPort module and reconfiguring is the only step
//                 needed to enable hardware connections.
class SerialLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit SerialLink(QString portName = QString(),
                        qint32 baudRate = 115200,
                        QObject* parent = nullptr);
    ~SerialLink() override;

    void    setPortName(const QString& portName);
    void    setBaudRate(qint32 baudRate);
    QString portName() const { return m_portName; }
    qint32  baudRate() const { return m_baudRate; }

    QString  name() const override;
    bool     isConnected() const override { return m_connected; }
    LinkKind kind() const override { return LinkKind::Serial; }

    // True iff the build was compiled against Qt6::SerialPort. When false,
    // connectLink() will fail with a clear errorOccurred message.
    static bool isBackendAvailable();

    // Enumerates the serial ports the host OS sees right now. Returns an
    // empty list when no SerialPort backend is compiled in.
    static QList<QString> availablePortNames();

    // Common baud rates we surface in the UI. Callers may set arbitrary
    // values via setBaudRate(); the list is just a convenience.
    static QList<qint32> commonBaudRates();

public slots:
    void connectLink() override;
    void disconnectLink() override;
    void writeBytes(const QByteArray& bytes) override;

private slots:
    void onReadyRead();

private:
    QString m_portName;
    qint32  m_baudRate;
    bool    m_connected = false;

#ifdef GCS_HAS_SERIAL_PORT
    std::unique_ptr<QSerialPort> m_serial;
#endif
};

} // namespace gcs::comms
