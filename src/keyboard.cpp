#include "keyboard.h"
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define test_bit(bit, array) (((array[(bit) / BITS_PER_LONG]) >> ((bit) % BITS_PER_LONG)) & 1UL)

Keyboard::Keyboard(QObject *parent) : QObject(parent) {
  m_scanTimer.setInterval(2500);
  connect(&m_scanTimer, &QTimer::timeout, this, &Keyboard::scanDevices);
  scanDevices();
  m_scanTimer.start();
}

Keyboard::~Keyboard() {
  closeDevices();
}

void Keyboard::closeDevices() {
  for (const auto &w : m_watchers) {
    if (w.notifier) {
      w.notifier->setEnabled(false);
      delete w.notifier;
    }
    if (w.fd >= 0)
      ::close(w.fd);
  }
  m_watchers.clear();
}

void Keyboard::dropWatcher(int fd) {
  for (int i = 0; i < m_watchers.size(); ++i) {
    if (m_watchers[i].fd != fd)
      continue;
    // Reached from the notifier's own signal: stop it now, delete it once the
    // handler has returned.
    m_watchers[i].notifier->setEnabled(false);
    m_watchers[i].notifier->deleteLater();
    ::close(fd);
    m_watchers.removeAt(i);
    return;
  }
}

void Keyboard::setActive(bool active) {
  if (m_active == active)
    return;
  m_active = active;
  for (const auto &w : m_watchers) {
    if (w.notifier)
      w.notifier->setEnabled(m_active);
  }
}

void Keyboard::scanDevices() {
  // Prune any dead watchers
  for (int i = m_watchers.size() - 1; i >= 0; --i) {
    if (m_watchers[i].fd < 0 || fcntl(m_watchers[i].fd, F_GETFD) == -1) {
      if (m_watchers[i].notifier) {
        m_watchers[i].notifier->setEnabled(false);
        delete m_watchers[i].notifier;
      }
      if (m_watchers[i].fd >= 0)
        ::close(m_watchers[i].fd);
      m_watchers.removeAt(i);
    }
  }

  QDir inputDir(QStringLiteral("/dev/input"));
  const QStringList entries = inputDir.entryList(QStringList() << QStringLiteral("event*"),
                                                 QDir::System);

  for (const QString &entry : entries) {
    const QString path = inputDir.filePath(entry);
    bool alreadyWatching = false;
    for (const auto &w : m_watchers) {
      if (w.path == path) {
        alreadyWatching = true;
        break;
      }
    }
    if (alreadyWatching)
      continue;

    int fd = ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
      continue;

    // Check EV_KEY capability
    unsigned long evBits[((EV_MAX - 1) / BITS_PER_LONG) + 1] = {0};
    if (::ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0) {
      ::close(fd);
      continue;
    }

    if (!test_bit(EV_KEY, evBits)) {
      ::close(fd);
      continue;
    }

    // Check whether it has standard keyboard keys
    unsigned long keyBits[((KEY_MAX - 1) / BITS_PER_LONG) + 1] = {0};
    if (::ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) < 0) {
      ::close(fd);
      continue;
    }

    if (test_bit(KEY_SPACE, keyBits) && test_bit(KEY_A, keyBits) && test_bit(KEY_ENTER, keyBits)) {
      qInfo() << "Nala: keyboard device connected:" << path;
      auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
      notifier->setEnabled(m_active);
      connect(notifier, &QSocketNotifier::activated, this, [this, fd] {
        struct input_event evs[64];
        while (true) {
          ssize_t bytes = ::read(fd, evs, sizeof(evs));
          if (bytes < 0 && errno == EINTR)
            continue;
          if (bytes < 0 && errno == EAGAIN)
            break;
          if (bytes <= 0) {
            // ENODEV: the device was unplugged or its node re-created. A dead
            // evdev fd stays "readable" forever and would spin the event loop,
            // so drop it; the next scan reopens whatever is there now.
            dropWatcher(fd);
            return;
          }
          const size_t count = bytes / sizeof(struct input_event);
          for (size_t i = 0; i < count; ++i) {
            // value == 1: key pressed (down)
            if (evs[i].type == EV_KEY && evs[i].value == 1) {
              emit keyPressed();
            }
          }
        }
      });
      m_watchers.append({path, fd, notifier});
    } else {
      ::close(fd);
    }
  }

  const bool nowAvailable = !m_watchers.isEmpty();
  if (nowAvailable != m_available) {
    m_available = nowAvailable;
    qInfo() << "Nala: keyboard listening active:" << m_available << "(found" << m_watchers.size() << "devices)";
    emit availableChanged();
  }
}
