#include "cursor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocalSocket>
#include <QProcessEnvironment>
#include <unistd.h>

namespace {
constexpr int kFastPollMs = 55;   // ~18 Hz: smooth for a glance, invisible in `top`
constexpr int kSlowPollMs = 1000; // 1 Hz: backoff retry when Hyprland is busy or suspended
constexpr int kConnectTimeoutMs = 60;
constexpr int kReadTimeoutMs = 60;
constexpr int kMaxFailuresBeforeSlow = 5;
} // namespace

QString Cursor::resolveSocket() {
  const auto env = QProcessEnvironment::systemEnvironment();
  const QString signature = env.value("HYPRLAND_INSTANCE_SIGNATURE");
  const QString runtime = env.value(
      "XDG_RUNTIME_DIR",
      QStringLiteral("/run/user/%1").arg(::getuid()));

  // 1. Direct instance signature if present and live
  if (!signature.isEmpty()) {
    const QString sock = runtime + "/hypr/" + signature + "/.socket.sock";
    if (QFile::exists(sock))
      return sock;
  }

  // 2. Scan $XDG_RUNTIME_DIR/hypr/ for newest instance directory containing .socket.sock
  const QDir hyprDir(runtime + "/hypr");
  if (hyprDir.exists()) {
    const QFileInfoList entries =
        hyprDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    for (const QFileInfo &entry : entries) {
      const QString candidate = entry.absoluteFilePath() + "/.socket.sock";
      if (QFile::exists(candidate))
        return candidate;
    }
  }

  return QString();
}

Cursor::Cursor(QObject *parent) : QObject(parent) {
  m_socket = resolveSocket();
  m_timer.setInterval(kFastPollMs);
  connect(&m_timer, &QTimer::timeout, this, &Cursor::poll);
}

void Cursor::setActive(bool active) {
  if (m_active == active && m_timer.isActive() == active)
    return;
  m_active = active;
  if (active) {
    if (m_socket.isEmpty() || !QFile::exists(m_socket))
      m_socket = resolveSocket();
    m_failures = 0;
    m_position = QPoint(-999999, -999999);
    m_timer.setInterval(kFastPollMs);
    m_timer.start();
    poll();
  } else {
    m_timer.stop();
    setAvailable(false);
  }
}

void Cursor::setAvailable(bool available) {
  if (m_available == available)
    return;
  m_available = available;
  emit availableChanged();
}

void Cursor::poll() {
  if (!m_active)
    return;

  // If socket path is unknown or no longer exists on disk, re-resolve
  if (m_socket.isEmpty() || !QFile::exists(m_socket)) {
    m_socket = resolveSocket();
    if (m_socket.isEmpty()) {
      if (++m_failures >= kMaxFailuresBeforeSlow) {
        setAvailable(false);
        m_timer.setInterval(kSlowPollMs);
      }
      return;
    }
  }

  QLocalSocket socket;
  socket.connectToServer(m_socket);
  if (!socket.waitForConnected(kConnectTimeoutMs)) {
    if (++m_failures >= kMaxFailuresBeforeSlow) {
      setAvailable(false);
      // Back off to slow interval instead of stopping dead: auto-recover as soon as Hyprland returns!
      m_timer.setInterval(kSlowPollMs);
      if (m_failures % 3 == 0)
        m_socket = resolveSocket();
    }
    return;
  }

  socket.write("cursorpos");
  socket.flush();
  if (!socket.waitForReadyRead(kReadTimeoutMs)) {
    if (++m_failures >= kMaxFailuresBeforeSlow) {
      setAvailable(false);
      m_timer.setInterval(kSlowPollMs);
    }
    return;
  }

  const QString reply = QString::fromUtf8(socket.readAll()).trimmed();
  const QStringList parts = reply.split(u',');
  if (parts.size() != 2)
    return;

  bool okX = false, okY = false;
  const int x = parts[0].trimmed().toInt(&okX);
  const int y = parts[1].trimmed().toInt(&okY);
  if (!okX || !okY)
    return;

  // Fast restore upon receiving any valid response
  if (m_failures > 0 || m_timer.interval() != kFastPollMs) {
    m_failures = 0;
    m_timer.setInterval(kFastPollMs);
  }
  setAvailable(true);

  const QPoint position(x, y);
  if (position == m_position)
    return;
  m_position = position;
  emit moved(m_position);
}

void Cursor::inject(QPoint position) {
  setAvailable(true);
  m_position = position;
  emit moved(m_position);
}

