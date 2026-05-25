#pragma once

#include <QFile>
#include <QString>
#include <memory>

#include "LogSink.h"

namespace gcs::logging {

// Appends one JSON object per line (JSONL) to a file. The file is opened
// once in `open()` and flushed after every write so crash diagnostics are
// preserved.
//
// SAFETY: This is a pure observer. It records what already happened — it
// does NOT influence any control path. If the file can't be opened the
// sink swallows the error and continues to no-op so logging never breaks
// the app.
class FileLogSink : public LogSink
{
public:
    FileLogSink();
    ~FileLogSink() override;

    // Returns true if the file was opened. On failure `lastError()` carries
    // the message. The sink stays usable (writes become no-ops).
    bool open(const QString& path);
    void close();

    QString filePath()  const { return m_path; }
    QString lastError() const { return m_lastError; }
    bool    isOpen()    const;

    void write(const LogEvent& e) override;

    // Convenience: returns the canonical log directory under the user's
    // AppLocalDataLocation, creating it if needed. Empty string on failure.
    static QString defaultLogDirectory();
    static QString defaultLogPathForNow();

private:
    QString               m_path;
    QString               m_lastError;
    std::unique_ptr<QFile> m_file;
};

} // namespace gcs::logging
