#pragma once

#include "LinkInterface.h"

namespace gcs::comms {

// In-memory loopback link for tests and simulation-mode bring-up.
//
// `writeBytes` is captured into `lastWrittenBytes()` so tests can assert what
// the caller transmitted. `injectIncomingBytes` lets tests simulate the
// other end of the wire. No real I/O occurs.
//
// In Phase 1 the MockVehicle drives VehicleState directly and does NOT push
// bytes through this link. The link still exists so the rest of the Comms /
// Protocol seam matches the real-link contract that Phase 3 will populate.
class MockLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit MockLink(QString name = QStringLiteral("MockLink"),
                      QObject* parent = nullptr);

    QString  name() const override { return m_name; }
    bool     isConnected() const override { return m_connected; }
    LinkKind kind() const override { return LinkKind::Mock; }

    QByteArray lastWrittenBytes() const { return m_lastWritten; }
    int writeCount() const { return m_writeCount; }
    void clearWriteHistory();

public slots:
    void connectLink() override;
    void disconnectLink() override;
    void writeBytes(const QByteArray& bytes) override;

    // Simulate bytes arriving from the "other end" of the link.
    void injectIncomingBytes(const QByteArray& bytes);

private:
    QString m_name;
    bool m_connected = false;
    QByteArray m_lastWritten;
    int m_writeCount = 0;
};

} // namespace gcs::comms
