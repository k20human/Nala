#include "backend.h"
#include "activity.h"
#include "compositor.h"
#include "cursor.h"
#include "mascot.h"
#include "music.h"
#include "theme.h"
#include "keyboard.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QQuickWindow>
#include <QRandomGenerator>
#include <iterator>
#include "mascot.h"
#include "music.h"
#include <QRegion>
#include <QSaveFile>
#include <QScreen>
#include <QStandardPaths>

#ifdef NALA_HAVE_LAYER_SHELL
#include <LayerShellQt/Window>
#endif

namespace {

constexpr qreal kMinSize = 0.55;
constexpr qreal kMaxSize = 2.20;

// The window is deliberately larger than the body so morphs that reach beyond
// the idle silhouette -- the exclamation mark, the orbit rings -- are not
// clipped. The reference reel uses the same headroom.
constexpr qreal kCanvasToBody = 1.89; // 1 / 0.529, the measured body radius
constexpr int kBaseBody = 116; // logical pixels at size 1.0

// Flight. Below this release speed she simply drops where she is put.
constexpr qreal kThrowSpeed = 900.0; // pixels per second
constexpr qreal kDrag = 1.5;         // per second
constexpr qreal kBounce = 0.62;      // energy kept when she hits an edge

QString autostartPath() {
  return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
         "/autostart/nala.desktop";
}

const QStringList kMochiThoughtsFR = {
    QStringLiteral("*bâille doucement* … 💤"),
    QStringLiteral("Zzz… un petit nuage passe… ☁️"),
    QStringLiteral("Un bon thé chaud ? 🍵"),
    QStringLiteral("Une petite gorgée d'eau… 💧"),
    QStringLiteral("*regarde par la fenêtre* 🍃"),
    QStringLiteral("Prends une grande inspiration… 🌸"),
    QStringLiteral("Douce journée avec toi… ✨"),
    QStringLiteral("*s'étire de tout son long* 🐾"),
    QStringLiteral("Toujours là à tes côtés. 🌿"),
    QStringLiteral("Un petit moment de calme. ☕"),
    QStringLiteral("Tu travailles bien aujourd'hui… 🤍"),
    QStringLiteral("Une petite pause pour tes yeux ? 🫧"),
    QStringLiteral("*ronronne doucement sur le bureau* ☁️"),
    QStringLiteral("Respire profondément… tout va bien. 🌱"),
    QStringLiteral("Je veille sur toi. ✨"),
    QStringLiteral("*observe curieusement ton écran* 👀"),
    QStringLiteral("Un doux rayon de soleil… ☀️"),
    QStringLiteral("*somnole paisiblement* 🌙"),
    QStringLiteral("Pense à relâcher tes épaules et ta nuque… 🌿"),
    QStringLiteral("Wheee ! Regarde comme je flotte ! 🎈"),
    QStringLiteral("N'oublie pas de cligner des yeux ! 👀✨"),
    QStringLiteral("Fais-toi confiance, tu vas y arriver. 🌸"),
    QStringLiteral("La météo est parfaite pour une pause câlin. 🐾"),
    QStringLiteral("*se roule en petite boule* 🧶"),
    QStringLiteral("Une gorgée d'eau fraîche, ça fait du bien ! 💧"),
    QStringLiteral("Écouter tes touches de clavier, c'est comme une musique… 🎶"),
    QStringLiteral("Prends le temps qu'il faut. Pas de stress. 🍃"),
    QStringLiteral("Tu as déjà accompli de belles choses aujourd'hui ! 🌟"),
    QStringLiteral("Une petite friandise ferait tellement plaisir… 🍪"),
    QStringLiteral("*te regarde avec affection* (◕ᴗ◕✿)"),
    QStringLiteral("C'est agréable d'être là avec toi. 🌿"),
    QStringLiteral("Pause étirements ? Lève les bras au ciel ! 🙆"),
    QStringLiteral("Ton bureau est si accueillant aujourd'hui… ✨"),
    QStringLiteral("*pousse un petit soupir de contentement* 💤"),
    QStringLiteral("Continue comme ça, pas à pas. 🐾")
};

const QStringList kMochiThoughtsEN = {
    QStringLiteral("*yawns softly* … 💤"),
    QStringLiteral("Zzz… drifting like a little cloud… ☁️"),
    QStringLiteral("How about a warm cup of tea? 🍵"),
    QStringLiteral("Remember to stay hydrated! 💧"),
    QStringLiteral("*gazes quietly out the window* 🍃"),
    QStringLiteral("Take a nice, deep breath… 🌸"),
    QStringLiteral("Wishing you a peaceful day… ✨"),
    QStringLiteral("*stretches paws out long* 🐾"),
    QStringLiteral("Right here beside you. 🌿"),
    QStringLiteral("A quiet, cozy moment. ☕"),
    QStringLiteral("You're doing great work today… 🤍"),
    QStringLiteral("Rest your eyes for a quick second? 🫧"),
    QStringLiteral("*purrs softly on your desk* ☁️"),
    QStringLiteral("Breathe in… everything is okay. 🌱"),
    QStringLiteral("I'm watching over you. ✨"),
    QStringLiteral("*curiously peeks at what you're doing* 👀"),
    QStringLiteral("A warm little ray of sunshine… ☀️"),
    QStringLiteral("*curled up snoozing peacefully* 🌙"),
    QStringLiteral("Remember to drop your shoulders and relax your neck… 🌿"),
    QStringLiteral("Wheee! Floating around! 🎈"),
    QStringLiteral("Don't forget to blink! 👀✨"),
    QStringLiteral("Believe in yourself, you've got this. 🌸"),
    QStringLiteral("The weather outside is perfect for a little break. 🐾"),
    QStringLiteral("*curls into a soft little ball* 🧶"),
    QStringLiteral("A cool sip of water works wonders! 💧"),
    QStringLiteral("The sound of your typing is like gentle music… 🎶"),
    QStringLiteral("Take your time. There's no rush. 🍃"),
    QStringLiteral("You've already accomplished so much today! 🌟"),
    QStringLiteral("A little snack would be wonderful right about now… 🍪"),
    QStringLiteral("*looks up at you lovingly* (◕ᴗ◕✿)"),
    QStringLiteral("It feels so nice spending time together. 🌿"),
    QStringLiteral("Time for a quick stretch? Reach for the sky! 🙆"),
    QStringLiteral("Your workspace feels so warm and cozy… ✨"),
    QStringLiteral("*gives a soft, contented sigh* 💤"),
    QStringLiteral("Keep going, one gentle step at a time. 🐾")
};

} // namespace

Backend::Backend(QString configPath, bool preview, bool testing, Mascot *mascot,
                 Theme *theme, Cursor *cursor, Activity *activity,
                 Compositor *compositor, Music *music, Keyboard *keyboard, QObject *parent)
    : QObject(parent), m_configPath(std::move(configPath)), m_preview(preview),
      m_testing(testing), m_mascot(mascot), m_theme(theme), m_cursor(cursor),
      m_activity(activity), m_compositor(compositor), m_music(music), m_keyboard(keyboard) {
  m_saveTimer.setSingleShot(true);
  m_saveTimer.setInterval(400); // coalesce slider drags into one write
  connect(&m_saveTimer, &QTimer::timeout, this, &Backend::save);

  if (m_keyboard) {
    connect(m_keyboard, &Keyboard::keyPressed, this, &Backend::onKeyPressed);
    connect(m_keyboard, &Keyboard::availableChanged, this, &Backend::changed);
  }

  load();
  applyToMascot();

  if (m_theme)
    connect(m_theme, &Theme::changed, this, [this] { emit changed(); });

  if (m_cursor && m_mascot) {
    connect(m_cursor, &Cursor::moved, this, [this](QPoint position) {
      if (m_laserActive) {
        const QRect screen = screenGeometry();
        m_toyPos = QPointF(position.x() - screen.x(), position.y() - screen.y());
        repositionToyWindow();
      }
      if (!m_followCursor && !m_laserActive)
        return;
      const qreal radius = bodyRadius();
      if (radius <= 0.0)
        return;
      const QPointF centre = centreOnScreen();
      m_mascot->lookAt((position.x() - centre.x()) / radius,
                       (position.y() - centre.y()) / radius);
    });
  }

  if (m_activity && m_mascot) {
    // A notification is what the badge has always been for.
    connect(m_activity, &Activity::notified, this, [this] {
      if (m_reactToDesktop) {
        m_mascot->notify();
        const bool inCorner = (m_place.x() > 0.75 && m_place.y() < 0.25);
        if (inCorner) {
          m_dodgeOffsetTarget = 1.0;
          m_dodgeTimer.start(6500); // Dodge to the left of the toast for 6.5 seconds
        }
        const QString app = m_activity->lastApp();
        const QString summary = m_activity->lastSummary();
        QString speech;
        if (!app.isEmpty() && app.toLower() != "notify-send") {
          if (m_language == "en") {
            speech = QStringLiteral("Oh! Notification from %1! 🔔").arg(app);
          } else {
            speech = QStringLiteral("Oh ! Une notification de %1 ! 🔔").arg(app);
          }
        } else if (!summary.isEmpty()) {
          if (m_language == "en") {
            speech = QStringLiteral("🔔 %1").arg(summary);
          } else {
            speech = QStringLiteral("🔔 %1 !").arg(summary);
          }
        } else {
          if (m_language == "en") {
            speech = QStringLiteral("Oh, you got a notification! 🔔✨");
          } else {
            speech = QStringLiteral("Oh, tu as reçu une notification ! 🔔✨");
          }
        }
        // When perched in the corner, wait for Nala to glide cleanly into position
        const int delay = inCorner ? 260 : 0;
        QTimer::singleShot(delay, this, [this, speech] {
          showSpeech(speech);
        });
      }
    });
  }

  if (m_compositor && m_mascot) {
    // Something went fullscreen: step aside rather than sitting on top of a
    // film. This is the one reaction that is about being considerate rather
    // than about being alive.
    connect(m_compositor, &Compositor::fullscreenChanged, this,
            [this](bool fullscreen) {
              if (m_outOfTheWay == fullscreen)
                return;
              m_outOfTheWay = fullscreen;
              applyInputRegion(false); // and stop swallowing clicks
              emit outOfTheWayChanged();
            });

    // You moved somewhere else, and she noticed.
    connect(m_compositor, &Compositor::workspaceChanged, this, [this] {
      if (m_outOfTheWay)
        return;
      m_mascot->wake();
      m_mascot->glanceAbout();
    });

    // Something new turned up.
    connect(m_compositor, &Compositor::windowOpened, this, [this] {
      if (m_outOfTheWay)
        return;
      m_mascot->wake();
      m_mascot->glanceAbout();
    });
  }

  if (m_music) {
    connect(m_music, &Music::playingChanged, this, [this](bool playing) {
      if (m_mascot)
        m_mascot->setSwaying(m_reactToDesktop && playing);
      emit musicPlayingChanged();
    });
    // A new track is worth looking up for.
    connect(m_music, &Music::trackChanged, this, [this] {
      if (m_reactToDesktop && m_music->playing() && m_mascot)
        m_mascot->glanceAbout();
    });
  }

  m_dodgeTimer.setSingleShot(true);
  connect(&m_dodgeTimer, &QTimer::timeout, this, [this] {
    m_dodgeOffsetTarget = 0.0;
  });

  // Outputs come and go in bursts — a dock waking up drops them all, then
  // brings them back one by one — so mapping Nala again waits for them to
  // settle rather than landing on a screen that is about to vanish.
  m_remapTimer.setSingleShot(true);
  m_remapTimer.setInterval(500);
  connect(&m_remapTimer, &QTimer::timeout, this, &Backend::remapToLiveScreen);

  if (!m_testing) {
    connect(qApp, &QGuiApplication::screenAdded, this,
            &Backend::screensChanged);
    connect(qApp, &QGuiApplication::screenRemoved, this,
            &Backend::screensChanged);
    // A screen turning up is when a dismissed Nala can come back.
    connect(qApp, &QGuiApplication::screenAdded, &m_remapTimer,
            qOverload<>(&QTimer::start));
  }
}

// --- preferences -----------------------------------------------------------

void Backend::load() {
  QFile file(m_configPath);
  if (!file.open(QIODevice::ReadOnly))
    return;
  const QJsonObject json =
      QJsonDocument::fromJson(file.readAll()).object();

  m_size = qBound(kMinSize, json.value("size").toDouble(m_size), kMaxSize);
  const QString mode = json.value("colorMode").toString(m_colorMode);
  if (mode == "ink" || mode == "theme" || mode == "custom" || mode == "gradient" || mode == "fur")
    m_colorMode = mode;
  m_customColor = json.value("customColor").toString(m_customColor);
  m_gradientColor = json.value("gradientColor").toString(m_gradientColor);
  const QString pattern = json.value("furPattern").toString(m_furPattern);
  if (pattern == "calico" || pattern == "tabby" || pattern == "leopard" ||
      pattern == "cow" || pattern == "siamese" || pattern == "tiger" || pattern == "dog")
    m_furPattern = pattern;
  m_dogFurSource = json.value("dogFurSource").toString(m_dogFurSource);
  if (m_dogFurSource != "theme" && m_dogFurSource != "custom" && m_dogFurSource != "gradient")
    m_dogFurSource = QStringLiteral("theme");
  const QString kReact = json.value("keyboardReaction").toString(m_keyboardReaction);
  if (kReact == "none" || kReact == "bounce" || kReact == "rings" || kReact == "pulse")
    m_keyboardReaction = kReact;
  m_followCursor = json.value("followCursor").toBool(m_followCursor);
  m_idleAntics = json.value("idleAntics").toBool(m_idleAntics);
  m_sleepWhenIdle = json.value("sleepWhenIdle").toBool(m_sleepWhenIdle);
  m_reducedMotion = json.value("reducedMotion").toBool(m_reducedMotion);
  m_reactToDesktop = json.value("reactToDesktop").toBool(m_reactToDesktop);
  m_stayOnTop = json.value("stayOnTop").toBool(m_stayOnTop);
  m_showTrayIcon = json.value("showTrayIcon").toBool(m_showTrayIcon);
  m_language = json.value("language").toString(m_language);
  if (m_language != "fr" && m_language != "en")
    m_language = QStringLiteral("fr");
  m_mochiPetting = json.value("mochiPetting").toBool(m_mochiPetting);
  m_mochiBubbles = json.value("mochiBubbles").toBool(m_mochiBubbles);
  m_mochiInterval = json.value("mochiInterval").toInt(m_mochiInterval);
  m_sinceThought = std::max(0, m_mochiInterval - 8);
  const QString acc = json.value("accessory").toString(m_accessory);
  if (acc == "none" || acc == "headphones" || acc == "sunglasses" ||
      acc == "glasses" || acc == "crown" || acc == "flower" || acc == "nightcap")
    m_accessory = acc;
  m_xp = std::max(0, json.value("xp").toInt(m_xp));
  if (m_mascot)
    m_mascot->setLevel(level());
  m_monitor = json.value("monitor").toString(m_monitor);
  m_place = QPointF(
      qBound(0.0, json.value("x").toDouble(m_place.x()), 1.0),
      qBound(0.0, json.value("y").toDouble(m_place.y()), 1.0));
}

void Backend::save() {
  if (m_testing)
    return; // a measured run must never overwrite real preferences

  QDir().mkpath(QFileInfo(m_configPath).absolutePath());
  const QJsonObject json{
      {"size", m_size},
      {"colorMode", m_colorMode},
      {"customColor", m_customColor},
      {"gradientColor", m_gradientColor},
      {"furPattern", m_furPattern},
      {"dogFurSource", m_dogFurSource},
      {"keyboardReaction", m_keyboardReaction},
      {"showTrayIcon", m_showTrayIcon},
      {"language", m_language},
      {"mochiPetting", m_mochiPetting},
      {"mochiBubbles", m_mochiBubbles},
      {"mochiInterval", m_mochiInterval},
      {"accessory", m_accessory},
      {"xp", m_xp},
      {"followCursor", m_followCursor},
      {"idleAntics", m_idleAntics},
      {"sleepWhenIdle", m_sleepWhenIdle},
      {"reducedMotion", m_reducedMotion},
      {"reactToDesktop", m_reactToDesktop},
      {"stayOnTop", m_stayOnTop},
      {"monitor", m_monitor},
      {"x", m_place.x()},
      {"y", m_place.y()},
  };

  QSaveFile file(m_configPath);
  if (!file.open(QIODevice::WriteOnly)) {
    note(QStringLiteral("Could not save preferences."));
    return;
  }
  file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
  if (!file.commit())
    note(QStringLiteral("Could not save preferences."));
}

void Backend::note(const QString &message) {
  m_feedback = message;
  emit feedbackChanged();
}

void Backend::applyToMascot() {
  if (!m_mascot)
    return;
  m_mascot->setLevel(level());
  m_mascot->setReducedMotion(m_reducedMotion);
  m_mascot->setSleepWhenIdle(m_sleepWhenIdle);
  m_mascot->setIdleAntics(m_idleAntics);
  if (m_cursor)
    m_cursor->setActive((m_followCursor || m_laserActive) && !m_testing);
  if (m_activity)
    m_activity->setActive(m_reactToDesktop && !m_testing);
  if (m_compositor)
    m_compositor->setActive(m_reactToDesktop && !m_testing);
  if (m_music) {
    m_music->setActive(m_reactToDesktop && !m_testing);
    if (m_mascot)
      m_mascot->setSwaying(m_reactToDesktop && m_music->playing());
  }
  if (!m_followCursor)
    m_mascot->lookIdle();
}

void Backend::configure(const QString &key, const QVariant &value) {
  if (key == "size") {
    const qreal wanted = value.toDouble();
    if (wanted < kMinSize || wanted > kMaxSize)
      return; // out of range: keep the current value rather than clamping
    m_size = wanted;
  } else if (key == "colorMode") {
    const QString mode = value.toString();
    if (mode != "ink" && mode != "theme" && mode != "custom" && mode != "gradient" && mode != "fur")
      return;
    m_colorMode = mode;
  } else if (key == "furPattern") {
    const QString pattern = value.toString();
    if (pattern != "calico" && pattern != "tabby" && pattern != "leopard" &&
        pattern != "cow" && pattern != "siamese" && pattern != "tiger" &&
        pattern != "dog")
      return;
    m_furPattern = pattern;
  } else if (key == "dogFurSource") {
    const QString src = value.toString();
    if (src != "theme" && src != "custom" && src != "gradient")
      return;
    m_dogFurSource = src;
  } else if (key == "customColor") {
    const QString c = value.toString();
    if (QColor::isValidColorName(c))
      m_customColor = c;
  } else if (key == "gradientColor") {
    const QString c = value.toString();
    if (QColor::isValidColorName(c))
      m_gradientColor = c;
  } else if (key == "keyboardReaction") {
    const QString mode = value.toString();
    if (mode != "none" && mode != "bounce" && mode != "rings" && mode != "pulse")
      return;
    m_keyboardReaction = mode;
  } else if (key == "followCursor") {
    m_followCursor = value.toBool();
  } else if (key == "idleAntics") {
    m_idleAntics = value.toBool();
  } else if (key == "sleepWhenIdle") {
    m_sleepWhenIdle = value.toBool();
  } else if (key == "reducedMotion") {
    m_reducedMotion = value.toBool();
  } else if (key == "reactToDesktop") {
    m_reactToDesktop = value.toBool();
  } else if (key == "stayOnTop") {
    m_stayOnTop = value.toBool();
  } else if (key == "showTrayIcon") {
    m_showTrayIcon = value.toBool();
    emit showTrayIconChanged();
  } else if (key == "language") {
    const QString l = value.toString();
    if (l == "fr" || l == "en") {
      m_language = l;
      emit languageChanged();
    }
  } else if (key == "mochiPetting") {
    m_mochiPetting = value.toBool();
    emit mochiPettingChanged();
  } else if (key == "mochiBubbles") {
    m_mochiBubbles = value.toBool();
    emit mochiBubblesChanged();
  } else if (key == "mochiInterval") {
    m_mochiInterval = std::max(30, value.toInt());
    m_sinceThought = std::min(m_sinceThought, qreal(m_mochiInterval));
    emit mochiIntervalChanged();
  } else if (key == "monitor") {
    m_monitor = value.toString();
  } else if (key == "accessory") {
    const QString acc = value.toString();
    if (acc != "none" && acc != "headphones" && acc != "sunglasses" &&
        acc != "glasses" && acc != "crown" && acc != "flower" && acc != "nightcap")
      return;
    m_accessory = acc;
    emit accessoryChanged();
  } else {
    return;
  }

  applyToMascot();
  applyPlacement();
  emit changed();
  m_saveTimer.start();
}

qreal Backend::furType() const {
  if (m_colorMode != "fur")
    return 0.0;
  if (m_furPattern == "calico")
    return 1.0;
  if (m_furPattern == "tabby")
    return 2.0;
  if (m_furPattern == "leopard")
    return 3.0;
  if (m_furPattern == "cow")
    return 4.0;
  if (m_furPattern == "siamese")
    return 5.0;
  if (m_furPattern == "tiger")
    return 6.0;
  if (m_furPattern == "dog")
    return 7.0;
  return 1.0;
}

QColor Backend::mascotColor() const {
  if (m_colorMode == "theme" && m_theme)
    return m_theme->mascotColor();
  if (m_colorMode == "custom" || m_colorMode == "gradient")
    return QColor(m_customColor);
  if (m_colorMode == "fur") {
    if (m_furPattern == "calico") return QColor("#e07a3c");
    if (m_furPattern == "tabby") return QColor("#9c7554");
    if (m_furPattern == "leopard") return QColor("#e59840");
    if (m_furPattern == "cow") return QColor("#4a4a4a");
    if (m_furPattern == "siamese") return QColor("#6d4c41");
    if (m_furPattern == "tiger") return QColor("#f3722c");
    if (m_furPattern == "dog") {
      if (m_dogFurSource == "theme" && m_theme)
        return m_theme->mascotColor();
      return QColor(m_customColor);
    }
    return QColor("#e07a3c");
  }
  return QColor("#0a090c"); // the reference silhouette
}

QColor Backend::mascotColor2() const {
  if (m_colorMode == "gradient")
    return QColor(m_gradientColor);
  if (m_colorMode == "fur") {
    if (m_furPattern == "calico") return QColor("#2b1d16");
    if (m_furPattern == "tabby") return QColor("#2b1d16");
    if (m_furPattern == "leopard") return QColor("#2b1d16");
    if (m_furPattern == "cow") return QColor("#141416");
    if (m_furPattern == "siamese") return QColor("#2b1d16");
    if (m_furPattern == "tiger") return QColor("#14110e");
    if (m_furPattern == "dog") {
      if (m_dogFurSource == "theme" && m_theme)
        return m_theme->mascotColor();
      if (m_dogFurSource == "gradient")
        return QColor(m_gradientColor);
      return QColor(m_customColor);
    }
  }
  return mascotColor();
}

bool Backend::keyboardAvailable() const {
  return m_keyboard && m_keyboard->available();
}

void Backend::onKeyPressed() {
  if (m_keyboardReaction == "bounce") {
    if (m_mascot)
      m_mascot->keyBounce();
  } else if (m_keyboardReaction == "rings") {
    if (m_mascot)
      m_mascot->keyRings();
  } else if (m_keyboardReaction == "pulse") {
    if (m_mascot)
      m_mascot->wake();
    m_keyPulse = 1.0;
    emit keyPulseChanged();
  }
}

// --- placement -------------------------------------------------------------

QStringList Backend::screens() const {
  QStringList names;
  for (const QScreen *screen : QGuiApplication::screens())
    names << screen->name();
  return names;
}

QRect Backend::screenGeometry() const {
  const QList<QScreen *> all = QGuiApplication::screens();
  for (QScreen *screen : all)
    if (!m_monitor.isEmpty() && screen->name() == m_monitor)
      return screen->geometry();
  QScreen *primary = QGuiApplication::primaryScreen();
  if (primary)
    return primary->geometry();
  return all.isEmpty() ? QRect(0, 0, 1920, 1080) : all.first()->geometry();
}

qreal Backend::windowSize() const {
  return kBaseBody * kCanvasToBody * m_size;
}

qreal Backend::bodyRadius() const {
  return windowSize() / kCanvasToBody * 0.5;
}

// Where Nala actually is, in compositor coordinates.
//
// Deliberately derived from the placement we asked for rather than from
// m_window->x()/y(): a layer-shell surface is positioned by the shell, and the
// QWindow's own coordinates do not describe where it ended up. Reading them
// here is what made the gaze aim at the wrong point.
QPointF Backend::centreOnScreen() const {
  // An ordinary window is placed by the compositor, so ask it where it ended
  // up. A layer-shell surface is not: its QWindow coordinates say nothing
  // about where the shell actually put it, so use the placement we asked for.
  if (m_window && !m_layered)
    return QPointF(m_window->x() + m_window->width() * 0.5,
                   m_window->y() + m_window->height() * 0.5);

  const QRect screen = screenGeometry();
  const qreal extent = windowSize();
  const qreal dodgePixels = m_dodgeOffset * 540.0;
  const qreal maxX = qreal(screen.width() - extent);
  const qreal rawX = m_place.x() * maxX - dodgePixels;
  const qreal clampedX = std::clamp(rawX, 0.0, maxX);
  return QPointF(
      screen.x() + clampedX + extent * 0.5,
      screen.y() + m_place.y() * (screen.height() - extent) + extent * 0.5);
}

void Backend::attach(QQuickWindow *window) {
  m_window = window;
  if (!m_window)
    return;

  m_window->setFlag(Qt::FramelessWindowHint, true);
  m_window->setColor(Qt::transparent);

#ifdef NALA_HAVE_LAYER_SHELL
  // Must happen while the window is still hidden: LayerShellQt can only give a
  // QWindow the layer-surface role before its platform surface exists. This is
  // what makes Nala a free-floating companion instead of a tiled window.
  if (!m_preview) {
    if (auto *layer = LayerShellQt::Window::get(m_window)) {
      layer->setScope(QStringLiteral("nala"));
      layer->setAnchors({LayerShellQt::Window::AnchorTop |
                         LayerShellQt::Window::AnchorLeft});
      layer->setExclusiveZone(-1); // never reserve space from other windows
      layer->setKeyboardInteractivity(
          LayerShellQt::Window::KeyboardInteractivityNone);
      // The compositor dismisses the surface when its output goes away. Left
      // open, the window would stay "visible" over a surface that no longer
      // exists; closed, it can be mapped again on a screen that does.
      layer->setCloseOnDismissed(true);
      m_layered = true;
    }
  }
#endif

  applyPlacement();
  m_window->setVisible(true);
  applyInputRegion(false);

  // Nala never hides her own window, so a layered one going hidden means the
  // compositor took her surface away.
  connect(m_window, &QWindow::visibleChanged, this, [this](bool visible) {
    if (!visible && m_layered)
      m_remapTimer.start();
  });
}

void Backend::remapToLiveScreen() {
  if (!m_window || m_window->isVisible())
    return;
  applyPlacement();
  m_window->setVisible(true);
  applyInputRegion(m_dragging);
}

void Backend::applyPlacement() {
  if (!m_window)
    return;

  const int extent = int(std::lround(windowSize()));
  const QRect screen = screenGeometry();
  const qreal dodgePixels = m_dodgeOffset * 540.0;
  const int maxX = screen.width() - extent;
  const int maxY = screen.height() - extent;
  const int x = std::clamp(int(std::lround(m_place.x() * maxX - dodgePixels)), 0, maxX);
  const int y = std::clamp(int(std::lround(m_place.y() * maxY)), 0, maxY);

#ifdef NALA_HAVE_LAYER_SHELL
  if (m_layered) {
    if (auto *layer = LayerShellQt::Window::get(m_window)) {
      QScreen *target = nullptr;
      for (QScreen *candidate : QGuiApplication::screens())
        if (!m_monitor.isEmpty() && candidate->name() == m_monitor)
          target = candidate;
      if (!target)
        target = QGuiApplication::primaryScreen();
      if (target)
        layer->setScreen(target);

      layer->setLayer(m_stayOnTop ? LayerShellQt::Window::LayerOverlay
                                  : LayerShellQt::Window::LayerBottom);
      // A layer surface is sized and placed by the shell, not by x/y.
      layer->setDesiredSize(QSize(extent, extent));
      layer->setMargins(QMargins(x, y, 0, 0));
      if (m_window->width() != extent || m_window->height() != extent)
        m_window->resize(extent, extent);
      applyInputRegion(m_dragging);
      if (m_bubbleWindow && m_bubbleWindow->isVisible())
        repositionBubble();
      if (m_snackWindow && m_snackWindow->isVisible())
        repositionSnackWindow();
      if (m_cursor && m_cursor->available() && m_mascot && (m_followCursor || m_laserActive)) {
        const qreal radius = bodyRadius();
        if (radius > 0.0) {
          const QPointF centre = centreOnScreen();
          const QPoint pos = m_cursor->position();
          m_mascot->lookAt((pos.x() - centre.x()) / radius,
                           (pos.y() - centre.y()) / radius);
        }
      }
      return;
    }
  }
#endif

  if (m_window->width() != extent || m_window->height() != extent)
    m_window->resize(extent, extent);
  m_window->setPosition(screen.x() + x, screen.y() + y);
  applyInputRegion(m_dragging);
  if (m_bubbleWindow && m_bubbleWindow->isVisible())
    repositionBubble();
  if (m_snackWindow && m_snackWindow->isVisible())
    repositionSnackWindow();
  if (m_cursor && m_cursor->available() && m_mascot && (m_followCursor || m_laserActive)) {
    const qreal radius = bodyRadius();
    if (radius > 0.0) {
      const QPointF centre = centreOnScreen();
      const QPoint pos = m_cursor->position();
      m_mascot->lookAt((pos.x() - centre.x()) / radius,
                       (pos.y() - centre.y()) / radius);
    }
  }
}

void Backend::applyInputRegion(bool wholeWindow) {
  if (!m_window)
    return;
  const int extent = m_window->width();
  if (extent <= 0)
    return;

  if (m_outOfTheWay) {
    // Stepped aside: take no input at all, or she would still be catching
    // clicks meant for whatever is fullscreen.
    m_window->setMask(QRegion());
    return;
  }

  if (wholeWindow) {
    // While she is being dragged the pointer wanders outside her silhouette;
    // widen the input region so the motion and the release still arrive.
    m_window->setMask(QRegion(0, 0, extent, extent));
    return;
  }

  // Otherwise only the mascot herself swallows clicks, so the transparent
  // margin around her stays click-through and never blocks the desktop.
  const int body = int(std::lround(extent / kCanvasToBody));
  const int inset = (extent - body) / 2;
  m_window->setMask(QRegion(inset, inset, body, body, QRegion::Ellipse));
}

void Backend::attachBubble(QQuickWindow *bubble) {
  m_bubbleWindow = bubble;
  if (!m_bubbleWindow)
    return;

  m_bubbleWindow->setFlag(Qt::FramelessWindowHint, true);
  m_bubbleWindow->setColor(Qt::transparent);

#ifdef NALA_HAVE_LAYER_SHELL
  if (!m_preview) {
    if (auto *layer = LayerShellQt::Window::get(m_bubbleWindow)) {
      layer->setScope(QStringLiteral("nala-bubble"));
      layer->setAnchors({LayerShellQt::Window::AnchorTop |
                         LayerShellQt::Window::AnchorLeft});
      layer->setExclusiveZone(-1);
      layer->setKeyboardInteractivity(
          LayerShellQt::Window::KeyboardInteractivityNone);
      layer->setCloseOnDismissed(false);
      layer->setLayer(LayerShellQt::Window::LayerOverlay);
      m_bubbleLayered = true;
    }
  }
#endif

  m_bubbleWindow->setVisible(false);
}

void Backend::repositionBubble() {
  if (!m_bubbleWindow)
    return;

  const QRect screen = screenGeometry();
  const qreal radius = bodyRadius();

  // Monitor-local coordinates of Nala's centre, including any dodge offset
  const qreal localX = centreOnScreen().x() - screen.x();
  const qreal localY = centreOnScreen().y() - screen.y();

  // Always the bubble's own size, never the window's current one: the shell
  // can configure a fresh layer surface to the whole usable area before its
  // first commit, and asking for that size back would make it stick.
  const int bw = 250;
  const int bh = 64;

  int bx = 0;
  int by = 0;
  bool isBelow = false;
  bool isAtRight = false;

  // When Nala is near the top-right corner or dodging notifications:
  const bool nearTopRight = (localX > screen.width() - 680 && localY < 400) || (m_dodgeOffset > 0.05);

  if (nearTopRight) {
    // Place bubble cleanly to the left of Nala, pointing right towards her
    const qreal gap = radius + 36.0;
    bx = int(localX - gap - bw);
    by = int(localY - bh * 0.5);
    isAtRight = true;
  } else {
    bx = int(std::clamp(localX - bw * 0.5, 12.0, qreal(screen.width() - bw - 12)));
    if (localY - radius - bh - 16 < 64.0) {
      by = int(localY + radius + 12);
      isBelow = true;
    } else {
      by = int(localY - radius - bh - 12);
      isBelow = false;
    }
  }

  bx = int(std::clamp(qreal(bx), 12.0, qreal(screen.width() - bw - 12)));
  by = int(std::clamp(qreal(by), 12.0, qreal(screen.height() - bh - 12)));

  const qreal arrowLocalX = std::clamp(localX - bx, 20.0, qreal(bw - 20));
  const qreal arrowLocalY = std::clamp(localY - by, 16.0, qreal(bh - 16));
  m_bubbleWindow->setProperty("arrowAtTop", isBelow);
  m_bubbleWindow->setProperty("arrowAtRight", isAtRight);
  m_bubbleWindow->setProperty("arrowX", arrowLocalX);
  m_bubbleWindow->setProperty("arrowY", arrowLocalY);

#ifdef NALA_HAVE_LAYER_SHELL
  if (m_bubbleLayered) {
    if (auto *layer = LayerShellQt::Window::get(m_bubbleWindow)) {
      QScreen *target = nullptr;
      for (QScreen *candidate : QGuiApplication::screens())
        if (!m_monitor.isEmpty() && candidate->name() == m_monitor)
          target = candidate;
      if (!target)
        target = QGuiApplication::primaryScreen();
      if (target)
        layer->setScreen(target);

      layer->setLayer(LayerShellQt::Window::LayerOverlay);
      layer->setDesiredSize(QSize(bw, bh));
      layer->setMargins(QMargins(bx, by, 0, 0));
    }
  }
#endif

  m_bubbleWindow->setPosition(screen.x() + bx, screen.y() + by);
}

void Backend::attachSnackWindow(QQuickWindow *snack) {
  m_snackWindow = snack;
  if (!m_snackWindow)
    return;

  m_snackWindow->setFlag(Qt::FramelessWindowHint, true);
  m_snackWindow->setColor(Qt::transparent);

#ifdef NALA_HAVE_LAYER_SHELL
  if (!m_preview) {
    if (auto *layer = LayerShellQt::Window::get(m_snackWindow)) {
      layer->setScope(QStringLiteral("nala-snacks"));
      layer->setAnchors({LayerShellQt::Window::AnchorTop |
                         LayerShellQt::Window::AnchorLeft});
      layer->setExclusiveZone(-1);
      layer->setKeyboardInteractivity(
          LayerShellQt::Window::KeyboardInteractivityNone);
      layer->setCloseOnDismissed(false);
      layer->setLayer(m_stayOnTop ? LayerShellQt::Window::LayerTop
                                  : LayerShellQt::Window::LayerBottom);
      m_snackLayered = true;
    }
  }
#endif

  m_snackWindow->setVisible(false);
}

void Backend::repositionSnackWindow() {
  if (!m_snackWindow)
    return;

  const QRect screen = screenGeometry();
  const int extent = int(std::lround(windowSize()));
  const qreal radius = bodyRadius();

  // Monitor-local coordinates of Nala's centre
  const qreal localX = m_place.x() * (screen.width() - extent) + extent * 0.5;
  const qreal localY = m_place.y() * (screen.height() - extent) + extent * 0.5;

  const int sw = m_snackWindow->width() > 0 ? m_snackWindow->width() : 290;
  const int sh = m_snackWindow->height() > 0 ? m_snackWindow->height() : 58;

  const int sx = int(std::clamp(localX - sw * 0.5, 12.0, qreal(screen.width() - sw - 12)));

  int sy = 0;
  bool isBelow = false;
  if (localY - radius - sh - 16 < 48.0) {
    sy = int(localY + radius + 12);
    isBelow = true;
  } else {
    sy = int(localY - radius - sh - 12);
    isBelow = false;
  }

  const qreal arrowLocalX = std::clamp(localX - sx, 18.0, qreal(sw - 18));
  m_snackWindow->setProperty("arrowAtTop", isBelow);
  m_snackWindow->setProperty("arrowX", arrowLocalX);

#ifdef NALA_HAVE_LAYER_SHELL
  if (m_snackLayered) {
    if (auto *layer = LayerShellQt::Window::get(m_snackWindow)) {
      QScreen *target = nullptr;
      for (QScreen *candidate : QGuiApplication::screens())
        if (!m_monitor.isEmpty() && candidate->name() == m_monitor)
          target = candidate;
      if (!target)
        target = QGuiApplication::primaryScreen();
      if (target)
        layer->setScreen(target);

      layer->setLayer(m_stayOnTop ? LayerShellQt::Window::LayerTop
                                  : LayerShellQt::Window::LayerBottom);
      layer->setDesiredSize(QSize(sw, sh));
      layer->setMargins(QMargins(sx, sy, 0, 0));
    }
  }
#endif

  m_snackWindow->setPosition(screen.x() + sx, screen.y() + sy);
}

void Backend::attachToyWindow(QQuickWindow *toy) {
  m_toyWindow = toy;
  if (!m_toyWindow)
    return;

  m_toyWindow->setFlag(Qt::FramelessWindowHint, true);
  m_toyWindow->setColor(Qt::transparent);
  m_toyWindow->setFlag(Qt::WindowTransparentForInput, true);
  m_toyWindow->setMask(QRegion(-1000, -1000, 1, 1));

#ifdef NALA_HAVE_LAYER_SHELL
  if (!m_preview) {
    if (auto *layer = LayerShellQt::Window::get(m_toyWindow)) {
      layer->setScope(QStringLiteral("nala-toy"));
      layer->setAnchors({LayerShellQt::Window::AnchorTop |
                         LayerShellQt::Window::AnchorLeft});
      layer->setExclusiveZone(-1);
      layer->setKeyboardInteractivity(
          LayerShellQt::Window::KeyboardInteractivityNone);
      layer->setCloseOnDismissed(false);
      layer->setLayer(LayerShellQt::Window::LayerOverlay);
      m_toyLayered = true;
    }
  }
#endif

  m_toyWindow->setVisible(false);
}

void Backend::repositionToyWindow() {
  if (!m_toyWindow || (!m_laserActive && !m_yarnActive))
    return;

  const QRect screen = screenGeometry();
  const int tw = m_toyWindow->width() > 0 ? m_toyWindow->width() : 72;
  const int th = m_toyWindow->height() > 0 ? m_toyWindow->height() : 72;

  int tx = 0;
  int ty = 0;
  if (m_laserActive) {
    const qreal lx = m_toyPos.x() - tw * 0.5;
    const qreal ly = m_toyPos.y() - th * 0.5;
    tx = std::clamp(int(std::lround(lx)), 0, screen.width() - tw);
    ty = std::clamp(int(std::lround(ly)), 0, screen.height() - th);
  } else {
    tx = std::clamp(int(std::lround(m_toyPos.x())), 0, screen.width() - tw);
    ty = std::clamp(int(std::lround(m_toyPos.y())), 0, screen.height() - th);
  }

#ifdef NALA_HAVE_LAYER_SHELL
  if (m_toyLayered) {
    if (auto *layer = LayerShellQt::Window::get(m_toyWindow)) {
      QScreen *target = nullptr;
      for (QScreen *candidate : QGuiApplication::screens())
        if (!m_monitor.isEmpty() && candidate->name() == m_monitor)
          target = candidate;
      if (!target)
        target = QGuiApplication::primaryScreen();
      if (target)
        layer->setScreen(target);

      layer->setLayer(LayerShellQt::Window::LayerOverlay);
      layer->setDesiredSize(QSize(tw, th));
      layer->setMargins(QMargins(tx, ty, 0, 0));
    }
  }
#endif

  m_toyWindow->setPosition(screen.x() + tx, screen.y() + ty);
}

void Backend::toyClicked() {
  if (m_yarnActive) {
    m_toyVel.setX((QRandomGenerator::global()->generateDouble() - 0.5) * 650.0);
    m_toyVel.setY(-420.0 - QRandomGenerator::global()->bounded(180));
    if (m_mascot)
      m_mascot->alert();
    addXp(15, QStringLiteral("toy"));
  }
}

void Backend::showSnackMenu() {
  if (!m_snackWindow || m_testing)
    return;
  repositionSnackWindow();
  m_snackWindow->setVisible(true);
  QMetaObject::invokeMethod(m_snackWindow, "appear");
}

void Backend::toggleSnackMenu() {
  if (!m_snackWindow || m_testing)
    return;
  const bool isOpen = m_snackWindow->property("isOpen").toBool();
  if (isOpen) {
    QMetaObject::invokeMethod(m_snackWindow, "dismiss");
  } else {
    showSnackMenu();
  }
}

void Backend::showSpeech(const QString &text) {
  if (!m_bubbleWindow || m_testing)
    return;

  QString msg = text;
  if (msg.isEmpty()) {
    const QStringList &list = (m_language == "en") ? kMochiThoughtsEN : kMochiThoughtsFR;
    if (list.isEmpty())
      return;
    msg = list.at(QRandomGenerator::global()->bounded(list.size()));
  }

  m_bubbleWindow->setProperty("message", msg);
  repositionBubble();
  m_bubbleWindow->setVisible(true);
  QMetaObject::invokeMethod(m_bubbleWindow, "appear");
}

void Backend::showRandomThought() {
  static int lastIdx = -1;
  const QTime now = QTime::currentTime();
  const int hour = now.hour();
  const int min = now.minute();

  const bool isNight = (hour >= 22 || hour < 6);
  const bool isEarlyMorning = (hour >= 6 && (hour < 8 || (hour == 8 && min <= 30)));
  const bool isLunch = (hour == 11 && min >= 45) || (hour == 12) || (hour == 13 && min <= 30);

  static const QStringList kNightThoughtsFR = {
    QStringLiteral("Zzz... J'ai tellement sommeil... 🥱💤"),
    QStringLiteral("Il se fait tard... tu ne dors pas encore ? 🌙"),
    QStringLiteral("Ouaaah... *bâille* mes petits yeux se ferment tout seuls... 💤"),
    QStringLiteral("Une petite pause dodo ? Mes moustaches tombent de sommeil... 🐾"),
    QStringLiteral("Bonne nuit... Je veille sur ton écran pendant que tu te reposes... 😴✨"),
    QStringLiteral("Pense à éteindre les écrans et aller au lit bientôt... 🌙✨")
  };

  static const QStringList kNightThoughtsEN = {
    QStringLiteral("Zzz... I'm so sleepy... 🥱💤"),
    QStringLiteral("It's getting late... bedtime soon? 🌙"),
    QStringLiteral("Yaaawn... my little eyes are closing on their own... 💤"),
    QStringLiteral("Nap time? My whiskers are drooping with sleep... 🐾"),
    QStringLiteral("Good night... I'll watch over your screen... 😴✨"),
    QStringLiteral("Don't forget to get some rest tonight... 🌙✨")
  };

  static const QStringList kEarlyMorningThoughtsFR = {
    QStringLiteral("Mmh... encore cinq minutes sous la couette... 🥱💤"),
    QStringLiteral("Déjà debout ? C'est bien trop tôt pour moi... ☕😴"),
    QStringLiteral("Ouaaah... *s'étire de tout son long* bonjour... 🥱🌸"),
    QStringLiteral("J'ai encore les yeux tout collés de sommeil... 😴"),
    QStringLiteral("Un bon chocolat chaud ou un café pour démarrer ? ☕✨"),
    QStringLiteral("Encore un peu la tête dans les nuages du matin... ☁️💤")
  };

  static const QStringList kEarlyMorningThoughtsEN = {
    QStringLiteral("Mmh... five more minutes in bed please... 🥱💤"),
    QStringLiteral("Up already? It's way too early for me... ☕😴"),
    QStringLiteral("Yaaawn... *stretches sleepily* good morning... 🥱🌸"),
    QStringLiteral("My eyes are still half shut... 😴"),
    QStringLiteral("Time for a hot tea or coffee to wake up? ☕✨"),
    QStringLiteral("Still dreaming in the morning clouds... ☁️💤")
  };

  static const QStringList kLunchThoughtsFR = {
    QStringLiteral("C'est l'heure de manger ! J'ai tellement faim ! 🍣🍱"),
    QStringLiteral("Mon petit ventre gargouille... qu'est-ce qu'on mange ce midi ? 🍙😋"),
    QStringLiteral("Miam miam ! Une pause déjeuner bien méritée ! 🍕✨"),
    QStringLiteral("Une petite friandise pour le midi ? S'il te plaît ! 🍡🥺"),
    QStringLiteral("À table ! Mes petites papilles frétillent d'avance ! 🍽️🐾"),
    QStringLiteral("Tu as prévu quoi de bon pour le déjeuner ? 🥗🥪")
  };

  static const QStringList kLunchThoughtsEN = {
    QStringLiteral("Lunchtime! I'm so hungry! 🍣🍱"),
    QStringLiteral("My tummy is rumbling... what are we having for lunch? 🍙😋"),
    QStringLiteral("Yum yum! Well-deserved lunch break! 🍕✨"),
    QStringLiteral("A little midday treat? Pretty please! 🍡🥺"),
    QStringLiteral("Time to eat! I'm starving! 🍽️🐾"),
    QStringLiteral("What's on the menu for lunch today? 🥗🥪")
  };

  QStringList pool;
  if (isNight) {
    pool = (m_language == "en") ? kNightThoughtsEN : kNightThoughtsFR;
  } else if (isEarlyMorning) {
    pool = (m_language == "en") ? kEarlyMorningThoughtsEN : kEarlyMorningThoughtsFR;
  } else if (isLunch) {
    pool = (m_language == "en") ? kLunchThoughtsEN : kLunchThoughtsFR;
  } else {
    pool = (m_language == "en") ? kMochiThoughtsEN : kMochiThoughtsFR;
  }

  if (pool.isEmpty())
    return;
  int idx = QRandomGenerator::global()->bounded(pool.size());
  if (idx == lastIdx && pool.size() > 1)
    idx = (idx + 1) % pool.size();
  lastIdx = idx;
  showSpeech(pool.at(idx));
}

bool Backend::musicPlaying() const {
  return m_music ? m_music->playing() : false;
}

void Backend::setAccessory(const QString &acc) {
  configure(QStringLiteral("accessory"), acc);
}

void Backend::toggleLaser() {
  setLaserActive(!m_laserActive);
}

void Backend::setLaserActive(bool active) {
  if (m_laserActive == active)
    return;
  m_laserActive = active;
  if (m_laserActive && m_yarnActive) {
    m_yarnActive = false;
    emit yarnActiveChanged();
  }
  emit laserActiveChanged();

  if (m_cursor) {
    m_cursor->setActive((m_followCursor || m_laserActive) && !m_testing);
  }

  if (m_toyWindow) {
    if (m_laserActive) {
      m_toyWindow->setFlag(Qt::WindowTransparentForInput, true);
      m_toyWindow->setMask(QRegion(-1000, -1000, 1, 1));
      const QRect screen = screenGeometry();
      if (m_cursor && m_cursor->available()) {
        m_toyPos = QPointF(m_cursor->position().x() - screen.x(),
                           m_cursor->position().y() - screen.y());
      } else {
        m_toyPos = QPointF(screen.width() * 0.5, screen.height() * 0.5);
      }
      m_toyWindow->setVisible(true);
      repositionToyWindow();
      if (m_mascot)
        m_mascot->alert();
      const bool fr = (m_language == "fr");
      showSpeech(fr ? QStringLiteral("Un point rouge ! Où est-il ?! 🔴👀") : QStringLiteral("A red dot! Where is it?! 🔴👀"));
    } else if (!m_yarnActive) {
      m_toyWindow->setVisible(false);
    }
  }
}

void Backend::toggleYarn() {
  setYarnActive(!m_yarnActive);
}

void Backend::setYarnActive(bool active) {
  if (m_yarnActive == active)
    return;
  m_yarnActive = active;
  if (m_yarnActive && m_laserActive) {
    m_laserActive = false;
    emit laserActiveChanged();
  }
  emit yarnActiveChanged();

  if (m_toyWindow) {
    if (m_yarnActive) {
      m_toyWindow->setFlag(Qt::WindowTransparentForInput, false);
      const QRect screen = screenGeometry();
      const QPointF centre = centreOnScreen();
      qreal sx = centre.x() - screen.x() + (centre.x() > screen.width() * 0.5 ? -140.0 : 140.0);
      qreal sy = centre.y() - screen.y() - 80.0;
      m_toyPos = QPointF(std::clamp(sx, 40.0, qreal(screen.width() - 100)),
                         std::clamp(sy, 40.0, qreal(screen.height() - 100)));
      m_toyVel = QPointF((centre.x() > screen.width() * 0.5 ? -240.0 : 240.0), -360.0);
      m_toyWindow->setVisible(true);
      const int tw = m_toyWindow->width() > 0 ? m_toyWindow->width() : 72;
      const int th = m_toyWindow->height() > 0 ? m_toyWindow->height() : 72;
      m_toyWindow->setMask(QRegion(0, 0, tw, th, QRegion::Ellipse));
      repositionToyWindow();
      if (m_mascot)
        m_mascot->alert();
      const bool fr = (m_language == "fr");
      showSpeech(fr ? QStringLiteral("Une pelote de laine ! À l'attaque ! 🧶") : QStringLiteral("A yarn ball! Attack! 🧶"));
    } else if (!m_laserActive) {
      m_toyWindow->setVisible(false);
    }
  }
}

void Backend::triggerPeekaboo() {
  setPeekabooActive(!m_peekabooActive);
  emit peekabooTriggered();
}

void Backend::setPeekabooActive(bool active) {
  if (m_peekabooActive == active)
    return;
  m_peekabooActive = active;
  emit peekabooActiveChanged();
  if (m_mascot) {
    if (m_peekabooActive) {
      m_mascot->sleepLull();
      const bool fr = (m_language == "fr");
      showSpeech(fr ? QStringLiteral("Où est Nala ? Cache-cache ! 🙈")
                    : QStringLiteral("Where is Nala? Peek-a-boo! 🙈"));
    } else {
      m_mascot->wake();
      m_mascot->love();
      addXp(15, QStringLiteral("peekaboo"));
      const bool fr = (m_language == "fr");
      showSpeech(fr ? QStringLiteral("Coucou ! Me voilà ! 🌸✨")
                    : QStringLiteral("Peek-a-boo ! Here I am ! 🌸✨"));
    }
  }
}

void Backend::celebrate() {
  if (m_mascot)
    m_mascot->celebrate();
  emit celebrateTriggered();
  const bool fr = (m_language == "fr");
  showSpeech(fr ? QStringLiteral("Hourra ! C'est la fête ! 🎉✨") : QStringLiteral("Hooray! Party time! 🎉✨"));
}

void Backend::tickle() {
  if (m_mascot)
    m_mascot->tickle();
  const bool fr = (m_language == "fr");
  showSpeech(fr ? QStringLiteral("Hihihi ! Ça chatouille ! 😂✨") : QStringLiteral("Hehehe! That tickles! 😂✨"));
}

void Backend::stretch() {
  if (m_mascot)
    m_mascot->stretchPurr();
  const bool fr = (m_language == "fr");
  showSpeech(fr ? QStringLiteral("Mmmh... C'est bon de s'étirer ! 🐾") : QStringLiteral("Mmmh... That stretch feels so good! 🐾"));
}

void Backend::sleep() {
  if (m_mascot)
    m_mascot->sleepLull();
  const bool fr = (m_language == "fr");
  showSpeech(fr ? QStringLiteral("Bonne nuit... Fais de beaux rêves... 💤") : QStringLiteral("Good night... Sweet dreams... 💤"));
}

void Backend::pet() {
  if (m_mascot)
    m_mascot->pet();
  if (m_timeSinceLastPetXp > 3.0) {
    m_timeSinceLastPetXp = 0.0;
    addXp(10, QStringLiteral("pet"));
  }
}

void Backend::love() {
  if (m_mascot)
    m_mascot->love();
}

void Backend::feed(const QString &snack) {
  if (m_mascot)
    m_mascot->feed(snack);
  emit snackRequested(snack);
  addXp(10, QStringLiteral("snack"));

  QTimer::singleShot(1600, this, [this, snack] {
    if (m_language == "en") {
      if (snack == "coffee")
        showSpeech(QStringLiteral("Warm, fresh coffee… ☕✨"));
      else if (snack == "fish")
        showSpeech(QStringLiteral("Yum, a tasty little fish! 🐟"));
      else if (snack == "donut")
        showSpeech(QStringLiteral("A sweet, delicious donut! 🍩✨"));
      else if (snack == "strawberry")
        showSpeech(QStringLiteral("A juicy, sweet strawberry! Yum! 🍓"));
      else if (snack == "tea")
        showSpeech(QStringLiteral("A soothing cup of warm tea… 🍵🌿"));
      else
        showSpeech(QStringLiteral("*munching on cookie* Delicious! 🍪"));
    } else {
      if (snack == "coffee")
        showSpeech(QStringLiteral("Un bon café bien chaud… ☕✨"));
      else if (snack == "fish")
        showSpeech(QStringLiteral("Miam, le petit poisson ! 🐟"));
      else if (snack == "donut")
        showSpeech(QStringLiteral("Un donut tout sucré et fondant ! 🍩✨"));
      else if (snack == "strawberry")
        showSpeech(QStringLiteral("Une petite fraise bien juteuse ! Miam ! 🍓"));
      else if (snack == "tea")
        showSpeech(QStringLiteral("Un thé bien chaud et apaisant… 🍵🌿"));
      else
        showSpeech(QStringLiteral("*croque le cookie* Délicieux ! 🍪"));
    }
  });
}

QString Backend::levelTitle() const {
  const int lvl = level();
  const bool fr = (m_language == "fr");
  if (lvl >= 10) return fr ? QStringLiteral("Divinité Féline 🌸") : QStringLiteral("Celestial Feline 🌸");
  switch (lvl) {
  case 1: return fr ? QStringLiteral("Chaton Curieux") : QStringLiteral("Curious Kitten");
  case 2: return fr ? QStringLiteral("Compagnon Joueur") : QStringLiteral("Playful Companion");
  case 3: return fr ? QStringLiteral("Étoile Brillante ⭐") : QStringLiteral("Shining Star ⭐");
  case 4: return fr ? QStringLiteral("Acrobate Aérien 🤸") : QStringLiteral("Aerial Acrobat 🤸");
  case 5: return fr ? QStringLiteral("Cœur Tendre 💖") : QStringLiteral("Tender Heart 💖");
  case 6: return fr ? QStringLiteral("Danseur Rythmique 💃") : QStringLiteral("Rhythmic Dancer 💃");
  case 7: return fr ? QStringLiteral("Nuage Rêveur ☁️") : QStringLiteral("Dreamy Cloud ☁️");
  case 8: return fr ? QStringLiteral("Semeur d'Étoiles ✨") : QStringLiteral("Star Sower ✨");
  case 9: return fr ? QStringLiteral("Harmonie Suprême 🎶") : QStringLiteral("Supreme Harmony 🎶");
  default: return fr ? QStringLiteral("Chaton Curieux") : QStringLiteral("Curious Kitten");
  }
}

QString Backend::nextUnlock() const {
  const int lvl = level();
  const bool fr = (m_language == "fr");
  if (lvl >= 10)
    return fr ? QStringLiteral("Niveau Maximum atteint ! 🎉") : QStringLiteral("Max Level reached! 🎉");
  switch (lvl) {
  case 1: return fr ? QStringLiteral("Niv. 2 : Réaction High-Five 🐾") : QStringLiteral("Lvl. 2: High-Five Reaction 🐾");
  case 2: return fr ? QStringLiteral("Niv. 3 : Métamorphose Étoile ⭐") : QStringLiteral("Lvl. 3: Star Form ⭐");
  case 3: return fr ? QStringLiteral("Niv. 4 : Salto Acrobatique 🤸") : QStringLiteral("Lvl. 4: Acrobatic Flip 🤸");
  case 4: return fr ? QStringLiteral("Niv. 5 : Métamorphose Cœur 💖") : QStringLiteral("Lvl. 5: Heart Form 💖");
  case 5: return fr ? QStringLiteral("Niv. 6 : Danse en Rythme 💃") : QStringLiteral("Lvl. 6: Dance Beat 💃");
  case 6: return fr ? QStringLiteral("Niv. 7 : Métamorphose Nuage ☁️") : QStringLiteral("Lvl. 7: Cloud Form ☁️");
  case 7: return fr ? QStringLiteral("Niv. 8 : Pluie d'Étoiles ✨") : QStringLiteral("Lvl. 8: Star Shower ✨");
  case 8: return fr ? QStringLiteral("Niv. 9 : Ronron Harmonique 🎶") : QStringLiteral("Lvl. 9: Harmonic Purr 🎶");
  case 9: return fr ? QStringLiteral("Niv. 10 : Forme Fleur Céleste 🌸") : QStringLiteral("Lvl. 10: Celestial Blossom Form 🌸");
  default: return QString();
  }
}

QString Backend::xpStatus() const {
  return QStringLiteral("Nala XP: %1 | Niveau %2 (%3) | Progression: %4/100 XP (%5%) | Prochain: %6\n")
      .arg(m_xp)
      .arg(level())
      .arg(levelTitle())
      .arg(xpInLevel())
      .arg(int(levelProgress() * 100))
      .arg(nextUnlock());
}

void Backend::addXp(int amount, const QString &source) {
  Q_UNUSED(source);
  if (amount <= 0)
    return;
  const int oldLevel = level();
  m_xp += amount;
  save();
  emit xpChanged();

  const int newLevel = level();
  if (newLevel > oldLevel) {
    if (m_mascot)
      m_mascot->setLevel(newLevel);
    emit levelChanged();

    QString rewardTitle;
    QString rewardDesc;
    const bool fr = (m_language == "fr");

    switch (newLevel) {
    case 2:
      rewardTitle = fr ? QStringLiteral("High-Five Débloqué ! 🐾") : QStringLiteral("High-Five Unlocked! 🐾");
      rewardDesc = fr ? QStringLiteral("Tape doucement dans la patte de Nala !") : QStringLiteral("Gently high-five Nala's paw!");
      break;
    case 3:
      rewardTitle = fr ? QStringLiteral("Forme Étoile Débloquée ! ⭐") : QStringLiteral("Star Form Unlocked! ⭐");
      rewardDesc = fr ? QStringLiteral("Nala peut se métamorphoser en étoile scintillante !") : QStringLiteral("Nala can now morph into a shining star!");
      break;
    case 4:
      rewardTitle = fr ? QStringLiteral("Salto Acrobatique ! 🤸") : QStringLiteral("Acrobatic Flip! 🤸");
      rewardDesc = fr ? QStringLiteral("Double-clique sur Nala pour lui faire faire un salto !") : QStringLiteral("Double-click Nala to do a 360° flip!");
      break;
    case 5:
      rewardTitle = fr ? QStringLiteral("Forme Cœur Débloquée ! 💖") : QStringLiteral("Heart Form Unlocked! 💖");
      rewardDesc = fr ? QStringLiteral("Nala peut se transformer en cœur débordant d'amour !") : QStringLiteral("Nala can morph into a heart full of love!");
      break;
    case 6:
      rewardTitle = fr ? QStringLiteral("Danse en Rythme ! 💃") : QStringLiteral("Dance Beat Unlocked! 💃");
      rewardDesc = fr ? QStringLiteral("Nala s'anime et danse au rythme de ta musique !") : QStringLiteral("Nala grooves to your music beats!");
      break;
    case 7:
      rewardTitle = fr ? QStringLiteral("Forme Nuage Débloquée ! ☁️") : QStringLiteral("Cloud Form Unlocked! ☁️");
      rewardDesc = fr ? QStringLiteral("Nala peut flotter comme un petit nuage vaporeux !") : QStringLiteral("Nala can morph into a fluffy cloud!");
      break;
    case 8:
      rewardTitle = fr ? QStringLiteral("Pluie d'Étoiles ! ✨") : QStringLiteral("Star Shower Unlocked! ✨");
      rewardDesc = fr ? QStringLiteral("Des étoiles magiques pleuvent autour de Nala !") : QStringLiteral("Magical shooting stars shower around Nala!");
      break;
    case 9:
      rewardTitle = fr ? QStringLiteral("Ronron Harmonique ! 🎶") : QStringLiteral("Harmonic Purr Unlocked! 🎶");
      rewardDesc = fr ? QStringLiteral("Des notes de musique dorées émanent de son ronronnement !") : QStringLiteral("Golden musical notes emanate from her purr!");
      break;
    case 10:
      rewardTitle = fr ? QStringLiteral("Fleur Céleste Suprême ! 🌸") : QStringLiteral("Celestial Blossom Form! 🌸");
      rewardDesc = fr ? QStringLiteral("Forme ultime atteinte ! Nala s'épanouit en fleur divine.") : QStringLiteral("Ultimate form reached! Nala blossoms into a divine flower.");
      break;
    default:
      rewardTitle = fr ? QStringLiteral("Nouveau Niveau ! 🎉") : QStringLiteral("Level Up! 🎉");
      rewardDesc = fr ? QStringLiteral("Nala continue d'évoluer avec toi !") : QStringLiteral("Nala continues to grow with you!");
      break;
    }

    emit leveledUp(newLevel, rewardTitle, rewardDesc);

    if (m_mascot)
      m_mascot->celebrate();
    showSpeech(QStringLiteral("🎉 %1\n%2").arg(rewardTitle, rewardDesc));
  }
}

void Backend::backflip() {
  if (level() < 4) {
    const bool fr = (m_language == "fr");
    showSpeech(fr ? QStringLiteral("Il me faut le Niveau 4 pour faire des saltos ! 🤸 (Actuel : Niv. %1)").arg(level())
                  : QStringLiteral("I need Level 4 to do flips! 🤸 (Current: Lvl. %1)").arg(level()));
    return;
  }
  if (m_mascot)
    m_mascot->backflip();
  addXp(5, QStringLiteral("flip"));
}

void Backend::highFive() {
  if (level() < 2) {
    const bool fr = (m_language == "fr");
    showSpeech(fr ? QStringLiteral("Tape m'en cinq sera disponible au Niveau 2 ! 🐾 (Actuel : Niv. %1)").arg(level())
                  : QStringLiteral("High-five will be unlocked at Level 2! 🐾 (Current: Lvl. %1)").arg(level()));
    return;
  }
  if (m_mascot)
    m_mascot->highFive();
  addXp(5, QStringLiteral("highfive"));
}

void Backend::starShower() {
  if (level() < 8) {
    const bool fr = (m_language == "fr");
    showSpeech(fr ? QStringLiteral("La pluie d'étoiles se débloque au Niveau 8 ! ✨ (Actuel : Niv. %1)").arg(level())
                  : QStringLiteral("Star shower unlocks at Level 8! ✨ (Current: Lvl. %1)").arg(level()));
    return;
  }
  if (m_mascot)
    m_mascot->starShower();
  addXp(5, QStringLiteral("starshower"));
}

void Backend::danceBeat() {
  if (level() < 6) {
    const bool fr = (m_language == "fr");
    showSpeech(fr ? QStringLiteral("La danse en rythme se débloque au Niveau 6 ! 💃 (Actuel : Niv. %1)").arg(level())
                  : QStringLiteral("Dance beat unlocks at Level 6! 💃 (Current: Lvl. %1)").arg(level()));
    return;
  }
  if (m_mascot)
    m_mascot->danceBeat();
  addXp(5, QStringLiteral("dance"));
}


void Backend::resetPlace() {
  m_place = QPointF(0.86, 0.74);
  applyPlacement();
  emit changed();
  m_saveTimer.start();
}

// --- drag ------------------------------------------------------------------

void Backend::grabDrag() {
  if (!m_window || m_dragging)
    return;
  m_dodgeOffset = 0.0;
  m_dodgeOffsetTarget = 0.0;
  m_dodgeTimer.stop();
  m_dragging = true;
  m_flying = false;
  m_dragSpeed = 0.0;
  m_dragVelocity = QPointF();
  m_dragClock.restart();
  applyInputRegion(true);
  if (m_mascot)
    m_mascot->beginDrag();
}

void Backend::dragBy(qreal dx, qreal dy) {
  if (!m_dragging || !m_window)
    return;

  const QRect screen = screenGeometry();
  const int extent = m_window->width();
  const qreal spanX = std::max(1, screen.width() - extent);
  const qreal spanY = std::max(1, screen.height() - extent);

  m_place = QPointF(qBound(0.0, m_place.x() + dx / spanX, 1.0),
                    qBound(0.0, m_place.y() + dy / spanY, 1.0));

  // Exponential average: a single jittery event should not read as a throw.
  m_dragSpeed = m_dragSpeed * 0.6 + std::hypot(dx, dy) * 0.4;

  // Velocity needs the interval between events, not just their size -- the
  // same 20 px means very different things 5 ms and 80 ms apart.
  const qreal elapsed =
      m_dragClock.isValid() ? m_dragClock.restart() / 1000.0 : 0.0;
  if (elapsed > 0.001 && elapsed < 0.2) {
    const QPointF sample(dx / elapsed, dy / elapsed);
    m_dragVelocity = m_dragVelocity * 0.55 + sample * 0.45;
  }

  applyPlacement();
  emit changed();
}

void Backend::releaseDrag() {
  if (!m_dragging)
    return;
  m_dragging = false;
  applyInputRegion(false);

  // Let go of her hard enough and she does not just drop -- she streaks off.
  const qreal speed = std::hypot(m_dragVelocity.x(), m_dragVelocity.y());
  if (speed > kThrowSpeed) {
    launch(m_dragVelocity.x(), m_dragVelocity.y(), speed);
    return;
  }

  if (m_mascot)
    m_mascot->endDrag(qBound(0.0, m_dragSpeed / 26.0, 1.0));
  m_saveTimer.start();
}

void Backend::launch(qreal dx, qreal dy, qreal speed) {
  if (!m_window)
    return;
  const QRect screen = screenGeometry();
  const int extent = m_window->width();
  const qreal spanX = std::max(1, screen.width() - extent);
  const qreal spanY = std::max(1, screen.height() - extent);

  m_flying = true;
  m_dragging = false;
  // Convert pixels per second into the place fractions the window moves in.
  m_flightVelocity = QPointF(dx / spanX, dy / spanY);
  if (m_mascot)
    m_mascot->beginDash(std::atan2(-dy, dx),
                        qBound(0.0, speed / (kThrowSpeed * 4.0), 1.0));
}

namespace {

// The demo reel: what she does, and how far into the run it happens.
struct DemoStep {
  qreal at;
  const char *what;
};
constexpr DemoStep kDemo[] = {
    {0.0, "wink"},   {2.6, "egg"},    {4.6, "hex"},    {6.6, "circle"},
    {7.6, "poke"},   {9.4, "think"},  {15.0, "alert"}, {17.8, "notify"},
    {21.8, "scatter"}, {24.4, "dash"}, {28.5, "rest"},
};

} // namespace

void Backend::demo() {
  if (!m_mascot)
    return;
  m_mascot->rest();
  m_demoStep = 0;
  m_demoClock = 0.0;
}

void Backend::advance(qreal dt) {
  // Demo playback. Driven from the same frame callback as everything else, so
  // it cannot drift out of step with her animation.
  if (m_demoStep >= 0 && m_mascot) {
    m_demoClock += dt;
    const int count = int(std::size(kDemo));
    while (m_demoStep < count && m_demoClock >= kDemo[m_demoStep].at) {
      const QString what = QString::fromLatin1(kDemo[m_demoStep].what);
      ++m_demoStep;
      if (what == "egg")
        m_mascot->changeForm(Mascot::Egg);
      else if (what == "hex")
        m_mascot->changeForm(Mascot::Hex);
      else if (what == "circle")
        m_mascot->changeForm(Mascot::Circle, Mascot::kSettleBack);
      else if (what == "dash")
        command("dash");
      else
        command(what);
    }
    if (m_demoStep >= count)
      m_demoStep = -1;
  }

  // She notices the machine working. Re-triggered on a cadence rather than
  // held, so a long build gets an occasional glance rather than a mascot
  // stuck in a permanent spin.
  if (m_reactToDesktop && m_activity && m_mascot) {
    m_sinceBusyThought += dt;
    if (m_activity->busy() && m_mascot->mood() == Mascot::Resting &&
        m_sinceBusyThought > 14.0) {
      m_sinceBusyThought = 0.0;
      m_mascot->think(3.4);
    }
  }

  // Asleep, she stirs when the cursor comes near rather than needing a poke.
  if (m_mascot && m_mascot->sleeping() && m_cursor && m_cursor->available()) {
    const qreal radius = bodyRadius();
    const QPointF centre = centreOnScreen();
    const QPointF to = QPointF(m_cursor->position()) - centre;
    if (radius > 0.0 && std::hypot(to.x(), to.y()) < radius * 3.0)
      m_mascot->wake();
  }

  if (m_keyPulse > 0.0) {
    m_keyPulse = std::max(0.0, m_keyPulse - 4.5 * dt);
    emit keyPulseChanged();
  }

  m_timeSinceLastPetXp += dt;
  if (!m_testing && m_activity && m_activity->isActive()) {
    m_activeTimeXpTimer += dt;
    if (m_activeTimeXpTimer >= 900.0) {
      m_activeTimeXpTimer = 0.0;
      addXp(10, QStringLiteral("active_time"));
    }
  }


  // Time-of-day fatigue (night / early morning sleepy state)
  if (!m_testing) {
    const QTime now = QTime::currentTime();
    const int hour = now.hour();
    const int min = now.minute();
    const bool isNight = (hour >= 22 || hour < 6);
    const bool isMorning = (hour >= 6 && (hour < 8 || (hour == 8 && min <= 30)));
    const bool fatigued = (isNight || isMorning);

    if (m_timeFatigued != fatigued) {
      m_timeFatigued = fatigued;
      emit timeFatiguedChanged();
    }

    if (m_mascot) {
      const qreal fatigueLevel = isNight ? 0.72 : (isMorning ? 0.55 : 0.0);
      m_mascot->setTimeFatigue(fatigueLevel);
    }
  }

  // Ambient Mochi companion thoughts
  if (m_mochiBubbles && !m_testing) {
    m_sinceThought += dt;
    if (m_sinceThought >= m_mochiInterval) {
      m_sinceThought = 0.0;
      if (m_mascot && !m_outOfTheWay) {
        if (m_mascot->sleeping())
          m_mascot->wake();
        showRandomThought();
      }
    }
  }

  // Animate dodging out of the way of notifications if perched in the top-right
  if (std::abs(m_dodgeOffset - m_dodgeOffsetTarget) > 0.001) {
    m_dodgeOffset += (m_dodgeOffsetTarget - m_dodgeOffset) * std::min(1.0, 14.0 * dt);
    applyPlacement();
  }

  // --- Toy Simulation (Laser & Yarn Ball) ---
  if (m_yarnActive) {
    const QRect screen = screenGeometry();
    const int tw = m_toyWindow ? m_toyWindow->width() : 72;
    const int th = m_toyWindow ? m_toyWindow->height() : 72;
    const qreal maxX = std::max(10.0, qreal(screen.width() - tw));
    const qreal maxY = std::max(10.0, qreal(screen.height() - th));

    m_toyVel.setY(m_toyVel.y() + 620.0 * dt);
    m_toyPos += m_toyVel * dt;
    m_toyRot += m_toyVel.x() * dt * 2.2;
    emit toyRotChanged();

    // Wall bounce
    if (m_toyPos.x() < 10.0) {
      m_toyPos.setX(10.0);
      m_toyVel.setX(-m_toyVel.x() * 0.78);
    } else if (m_toyPos.x() > maxX - 10.0) {
      m_toyPos.setX(maxX - 10.0);
      m_toyVel.setX(-m_toyVel.x() * 0.78);
    }

    // Floor bounce
    if (m_toyPos.y() > maxY - 10.0) {
      m_toyPos.setY(maxY - 10.0);
      m_toyVel.setY(-m_toyVel.y() * 0.75);
      m_toyVel.setX(m_toyVel.x() * 0.94);
      if (std::abs(m_toyVel.y()) < 30.0) {
        m_toyVel.setY(-200.0 - QRandomGenerator::global()->bounded(160));
        m_toyVel.setX((QRandomGenerator::global()->generateDouble() - 0.5) * 380.0);
      }
    } else if (m_toyPos.y() < 10.0) {
      m_toyPos.setY(10.0);
      m_toyVel.setY(-m_toyVel.y() * 0.75);
    }

    // Nala watches the yarn ball and chases it
    const QPointF centre = centreOnScreen();
    const qreal radius = bodyRadius();
    if (m_mascot && radius > 0.0) {
      const qreal toToyX = (screen.x() + m_toyPos.x() + tw * 0.5 - centre.x());
      const qreal toToyY = (screen.y() + m_toyPos.y() + th * 0.5 - centre.y());
      m_mascot->lookAt(toToyX / radius, toToyY / radius);

      const qreal dist = std::hypot(toToyX, toToyY);
      // Nala bats the ball if it touches her
      if (dist < radius + 40.0) {
        m_toyVel.setX((toToyX > 0 ? 1.0 : -1.0) * (260.0 + QRandomGenerator::global()->bounded(200)));
        m_toyVel.setY(-320.0 - QRandomGenerator::global()->bounded(160));
        m_mascot->pounce(toToyX > 0 ? 0.7 : -0.7, -0.5);
      }

      // Periodically Nala pounces playfully towards the yarn ball
      m_timeSinceToyPounce += dt;
      if (m_timeSinceToyPounce > 4.0 && !m_flying) {
        m_timeSinceToyPounce = 0.0;
        m_mascot->pounce(toToyX / radius, toToyY / radius);
      }
    }

    repositionToyWindow();
  } else if (m_laserActive) {
    const QRect screen = screenGeometry();
    const QPointF centre = centreOnScreen();
    const qreal radius = bodyRadius();

    if (m_mascot && radius > 0.0) {
      const qreal toLaserX = (screen.x() + m_toyPos.x() - centre.x());
      const qreal toLaserY = (screen.y() + m_toyPos.y() - centre.y());
      m_mascot->lookAt(toLaserX / radius, toLaserY / radius);

      m_timeSinceToyPounce += dt;
      if (m_timeSinceToyPounce > 2.5 && !m_flying) {
        m_timeSinceToyPounce = 0.0;
        m_mascot->pounce(toLaserX / radius, toLaserY / radius);
      }
    }
  }

  if (!m_flying || !m_window)
    return;

  const QRect screen = screenGeometry();
  const int extent = m_window->width();
  const qreal spanX = std::max(1.0, qreal(screen.width() - extent));
  const qreal spanY = std::max(1.0, qreal(screen.height() - extent));

  QPointF place = m_place + m_flightVelocity * dt;

  // Bounce off the edges of the screen rather than sticking to them.
  if (place.x() < 0.0 || place.x() > 1.0) {
    place.setX(qBound(0.0, place.x() < 0.0 ? -place.x() : 2.0 - place.x(), 1.0));
    m_flightVelocity.setX(-m_flightVelocity.x() * kBounce);
  }
  if (place.y() < 0.0 || place.y() > 1.0) {
    place.setY(qBound(0.0, place.y() < 0.0 ? -place.y() : 2.0 - place.y(), 1.0));
    m_flightVelocity.setY(-m_flightVelocity.y() * kBounce);
  }
  m_place = place;

  // Air resistance.
  m_flightVelocity *= std::max(0.0, 1.0 - kDrag * dt);

  const qreal pixelsPerSecond =
      std::hypot(m_flightVelocity.x() * spanX, m_flightVelocity.y() * spanY);
  if (m_mascot)
    m_mascot->updateDash(
        std::atan2(-m_flightVelocity.y() * spanY, m_flightVelocity.x() * spanX),
        qBound(0.0, pixelsPerSecond / (kThrowSpeed * 4.0), 1.0));

  applyPlacement();
  emit changed();

  if (pixelsPerSecond < kThrowSpeed * 0.35) {
    m_flying = false;
    m_flightVelocity = QPointF();
    if (m_mascot)
      m_mascot->endDash();
    m_saveTimer.start();
  }
}

void Backend::injectCursor(int x, int y) {
  if (m_cursor)
    m_cursor->inject(QPoint(x, y));
}

// --- commands --------------------------------------------------------------

void Backend::openSettings() { emit settingsRequested(); }

void Backend::quit() { QCoreApplication::quit(); }

void Backend::command(const QString &name) {
  if (!m_mascot)
    return;
  if (name == "poke")
    m_mascot->poke();
  else if (name == "think")
    m_mascot->think();
  else if (name == "alert")
    m_mascot->alert();
  else if (name == "notify")
    m_mascot->notify();
  else if (name == "wake")
    m_mascot->wake();
  else if (name == "rest")
    m_mascot->rest();
  else if (name == "wink")
    m_mascot->wink();
  else if (name == "scatter")
    m_mascot->scatter();
  else if (name == "pet")
    pet();
  else if (name == "love" || name == "heart")
    love();
  else if (name.startsWith("feed")) {
    const QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    const QString snack = parts.size() > 1 ? parts[1] : QStringLiteral("cookie");
    feed(snack);
  }
  else if (name.startsWith("say ")) {
    showSpeech(name.mid(4).trimmed());
  }
  else if (name == "talk") {
    showRandomThought();
  }
  else if (name == "demo")
    demo();
  else if (name == "dash") {
    // Off in some direction of her own choosing.
    const qreal angle = QRandomGenerator::global()->generateDouble() * 2 * M_PI;
    launch(std::cos(angle) * kThrowSpeed * 2.2,
           std::sin(angle) * kThrowSpeed * 2.2, kThrowSpeed * 2.2);
  }
  else if (name == "snack" || name == "snacks") {
    toggleSnackMenu();
  }
  else if (name == "settings" || name == "preferences" || name == "config")
    openSettings();
  else if (name == "reset")
    resetPlace();
  else if (name.startsWith("fur")) {
    const QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    if (parts.size() > 1) {
      configure("furPattern", parts[1]);
      if (parts[1] == "dog" && parts.size() > 2) {
        if (parts[2] == "theme" || parts[2] == "custom" || parts[2] == "gradient")
          configure("dogFurSource", parts[2]);
        if (parts[2] == "custom" && parts.size() > 3 && QColor::isValidColorName(parts[3])) {
          configure("customColor", parts[3]);
        } else if (parts[2] == "gradient" && parts.size() > 4 &&
                   QColor::isValidColorName(parts[3]) && QColor::isValidColorName(parts[4])) {
          configure("customColor", parts[3]);
          configure("gradientColor", parts[4]);
        }
      }
    }
    configure("colorMode", "fur");
  }
  else if (name.startsWith("color")) {
    const QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    if (parts.size() == 2) {
      if (parts[1] == "ink" || parts[1] == "theme" || parts[1] == "fur") {
        configure("colorMode", parts[1]);
      } else if (parts[1] == "calico" || parts[1] == "tabby" || parts[1] == "leopard" ||
                 parts[1] == "cow" || parts[1] == "siamese" || parts[1] == "tiger" ||
                 parts[1] == "dog") {
        configure("furPattern", parts[1]);
        configure("colorMode", "fur");
      } else if (QColor::isValidColorName(parts[1])) {
        configure("customColor", parts[1]);
        configure("colorMode", "custom");
      }
    } else if (parts.size() >= 3) {
      if (parts[1] == "fur") {
        configure("furPattern", parts[2]);
        if (parts[2] == "dog" && parts.size() > 3) {
          if (parts[3] == "theme" || parts[3] == "custom" || parts[3] == "gradient")
            configure("dogFurSource", parts[3]);
          if (parts[3] == "custom" && parts.size() > 4 && QColor::isValidColorName(parts[4])) {
            configure("customColor", parts[4]);
          } else if (parts[3] == "gradient" && parts.size() > 5 &&
                     QColor::isValidColorName(parts[4]) && QColor::isValidColorName(parts[5])) {
            configure("customColor", parts[4]);
            configure("gradientColor", parts[5]);
          }
        }
        configure("colorMode", "fur");
      } else if (QColor::isValidColorName(parts[1]) && QColor::isValidColorName(parts[2])) {
        configure("customColor", parts[1]);
        configure("gradientColor", parts[2]);
        configure("colorMode", "gradient");
      }
    }
  }
  else if (name.startsWith("hat") || name.startsWith("accessory")) {
    const QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    if (parts.size() > 1) {
      configure("accessory", parts[1]);
    } else {
      static const QStringList hats = {"none", "headphones", "sunglasses", "glasses", "crown", "flower", "nightcap"};
      int idx = hats.indexOf(m_accessory);
      int nextIdx = (idx + 1) % hats.size();
      configure("accessory", hats[nextIdx]);
    }
  }
  else if (name == "laser") {
    toggleLaser();
  }
  else if (name == "yarn" || name == "ball") {
    toggleYarn();
  }
  else if (name == "peekaboo" || name == "coucou") {
    triggerPeekaboo();
  }
  else if (name == "celebrate" || name == "party" || name == "fete") {
    celebrate();
  }
  else if (name == "tickle" || name == "chatouille") {
    tickle();
  }
  else if (name == "pancake" || name == "crepe" || name == "squash") {
    if (m_mascot)
      m_mascot->pancakeSquash();
    const bool fr = m_language == QStringLiteral("fr");
    showSpeech(fr ? QStringLiteral("Plaf ! Je suis une crêpe ! 🥞") : QStringLiteral("Splat! I'm a pancake! 🥞"));
  }
  else if (name == "stretch" || name == "etire") {
    stretch();
  }
  else if (name == "sleep" || name == "dodo") {
    sleep();
  }
  else if (name == "xp" || name == "level") {
    showSpeech(QStringLiteral("⭐ Niveau %1 (%2) — %3/100 XP\n%4").arg(QString::number(level()), levelTitle(), QString::number(xpInLevel()), nextUnlock()));
  }
  else if (name.startsWith("add-xp ") || name.startsWith("addxp ")) {
    const QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    if (parts.size() > 1) {
      bool ok = false;
      int amount = parts[1].toInt(&ok);
      if (ok && amount > 0)
        addXp(amount, QStringLiteral("cli"));
    }
  }
  else if (name == "backflip" || name == "flip" || name == "salto") {
    backflip();
  }
  else if (name == "highfive" || name == "high-five") {
    highFive();
  }
  else if (name == "starshower" || name == "stars") {
    starShower();
  }
  else if (name == "dance") {
    danceBeat();
  }
  else if (name == "quit")
    quit();
}


QString Backend::status() const {
  static const char *moods[] = {"resting",   "happy",  "thinking", "alert",
                                "notifying", "asleep", "held",     "dashing"};
  const int mood = m_mascot ? m_mascot->mood() : 0;
  const int moodCount = int(std::size(moods));
  QString modeDesc = m_colorMode == "theme" ? QStringLiteral("following the desktop theme")
                     : m_colorMode == "gradient" ? QStringLiteral("gradient (%1 -> %2)").arg(m_customColor, m_gradientColor)
                     : m_colorMode == "custom" ? QStringLiteral("custom (%1)").arg(m_customColor)
                     : m_colorMode == "fur" ? QStringLiteral("fur (%1)").arg(m_furPattern)
                     : QStringLiteral("ink");
  return QStringLiteral("Nala is running — size %1%, %2, at %3%/%4%, %5, %6 "
                        "(form %7→%8 %9%)%10")
      .arg(int(std::lround(m_size * 100)))
      .arg(modeDesc)
      .arg(int(std::lround(m_place.x() * 100)))
      .arg(int(std::lround(m_place.y() * 100)))
      .arg(mood >= 0 && mood < moodCount ? moods[mood] : "?")
      .arg(!m_followCursor        ? "gaze free"
           : m_cursor && m_cursor->available()
               ? "tracking the cursor"
               : "cursor unavailable")
      .arg(m_mascot ? m_mascot->formA() : -1)
      .arg(m_mascot ? m_mascot->formB() : -1)
      .arg(m_mascot ? int(std::lround(m_mascot->formMix() * 100)) : 0)
      .arg(m_mascot && m_mascot->swaying() ? ", swaying" : "");
}

// --- autostart -------------------------------------------------------------

bool Backend::startAtLogin() const { return QFile::exists(autostartPath()); }

void Backend::setStartAtLogin(bool enabled) {
  const QString path = autostartPath();
  if (!enabled) {
    if (QFile::exists(path) && !QFile::remove(path))
      note(QStringLiteral("Could not remove the autostart entry."));
    emit changed();
    return;
  }

  QDir().mkpath(QFileInfo(path).absolutePath());
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    note(QStringLiteral("Could not create the autostart entry."));
    emit changed();
    return;
  }
  const QString entry = QStringLiteral("[Desktop Entry]\n"
                                       "Type=Application\n"
                                       "Name=Nala\n"
                                       "Comment=Desktop companion\n"
                                       "Exec=%1\n"
                                       "Terminal=false\n"
                                       "X-GNOME-Autostart-enabled=true\n")
                            .arg(QCoreApplication::applicationFilePath());
  file.write(entry.toUtf8());
  if (!file.commit())
    note(QStringLiteral("Could not create the autostart entry."));
  emit changed();
}
