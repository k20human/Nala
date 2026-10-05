#pragma once
#include <QObject>
#include <QSocketNotifier>
#include <QTimer>
#include <QVector>

class Keyboard : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
  explicit Keyboard(QObject *parent = nullptr);
  ~Keyboard() override;

  bool available() const { return m_available; }
  void setActive(bool active);

signals:
  void availableChanged();
  void keyPressed();

private slots:
  void scanDevices();

private:
  void closeDevices();
  void dropWatcher(int fd);

  bool m_active = true;
  bool m_available = false;
  QTimer m_scanTimer;

  struct Watcher {
    QString path;
    int fd = -1;
    QSocketNotifier *notifier = nullptr;
  };
  QVector<Watcher> m_watchers;
};
