#pragma once
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

// What the machine is up to, so Nala has something to react to besides a
// timer. Two signals: whether the system is working hard, and whether a
// notification just arrived.
//
// Load comes from /proc/loadavg. Notifications come from Noctalia's own
// history file, which is the notification daemon on the desktop this is built
// for; without it she simply never reports one.
class Activity : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
  Activity(QString stateRoot, QObject *parent = nullptr);

  bool busy() const { return m_busy; }
  bool watchingNotifications() const {
    return !m_historyPath.isEmpty() || !m_omarchyNotificationsDir.isEmpty();
  }

  QString lastApp() const { return m_lastApp; }
  QString lastSummary() const { return m_lastSummary; }

  void setActive(bool active);
  bool isActive() const { return m_active; }

  // Test seams.
  void injectLoad(qreal perCore);
  void injectNotification();

signals:
  void busyChanged(bool busy);
  void notified();

private:
  void pollLoad();
  void readHistory();
  void readOmarchyNotifications();

  QString m_historyPath;
  QString m_omarchyNotificationsDir;
  QString m_omarchyHistoryDir;
  QString m_lastApp;
  QString m_lastSummary;
  qint64 m_lastOmarchyTimestamp = -1;

  QTimer m_loadTimer;
  QFileSystemWatcher m_watcher;
  QTimer m_debounce;
  qint64 m_lastSerial = -1;
  int m_cores = 1;
  bool m_busy = false;
  bool m_active = false;
};
