#pragma once
#include <QColor>
#include <QJsonObject>
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QStringList>
#include <QElapsedTimer>
#include <QTimer>

class QQuickWindow;
class Mascot;
class Theme;
class Cursor;
class Activity;
class Compositor;
class Music;
class Keyboard;

// Preferences, placement and the glue between the mascot and the desktop.
class Backend : public QObject {
  Q_OBJECT

  Q_PROPERTY(qreal size READ size NOTIFY changed)
  Q_PROPERTY(QString colorMode READ colorMode NOTIFY changed)
  Q_PROPERTY(QColor mascotColor READ mascotColor NOTIFY changed)
  Q_PROPERTY(QColor mascotColor2 READ mascotColor2 NOTIFY changed)
  Q_PROPERTY(QString customColor READ customColor NOTIFY changed)
  Q_PROPERTY(QString gradientColor READ gradientColor NOTIFY changed)
  Q_PROPERTY(QString furPattern READ furPattern NOTIFY changed)
  Q_PROPERTY(qreal furType READ furType NOTIFY changed)
  Q_PROPERTY(QString dogFurSource READ dogFurSource NOTIFY changed)
  Q_PROPERTY(bool followCursor READ followCursor NOTIFY changed)
  Q_PROPERTY(bool idleAntics READ idleAntics NOTIFY changed)
  Q_PROPERTY(bool reactToDesktop READ reactToDesktop NOTIFY changed)
  Q_PROPERTY(bool sleepWhenIdle READ sleepWhenIdle NOTIFY changed)
  Q_PROPERTY(bool reducedMotion READ reducedMotion NOTIFY changed)
  Q_PROPERTY(bool stayOnTop READ stayOnTop NOTIFY changed)
  Q_PROPERTY(bool startAtLogin READ startAtLogin NOTIFY changed)
  Q_PROPERTY(bool showTrayIcon READ showTrayIcon NOTIFY showTrayIconChanged)
  Q_PROPERTY(QString language READ language NOTIFY languageChanged)
  Q_PROPERTY(bool mochiPetting READ mochiPetting NOTIFY mochiPettingChanged)
  Q_PROPERTY(bool mochiBubbles READ mochiBubbles NOTIFY mochiBubblesChanged)
  Q_PROPERTY(int mochiInterval READ mochiInterval NOTIFY mochiIntervalChanged)
  Q_PROPERTY(QString keyboardReaction READ keyboardReaction NOTIFY changed)
  Q_PROPERTY(bool keyboardAvailable READ keyboardAvailable NOTIFY changed)
  Q_PROPERTY(qreal keyPulse READ keyPulse NOTIFY keyPulseChanged)
  Q_PROPERTY(QString accessory READ accessory WRITE setAccessory NOTIFY accessoryChanged)
  Q_PROPERTY(bool musicPlaying READ musicPlaying NOTIFY musicPlayingChanged)
  Q_PROPERTY(bool laserActive READ laserActive WRITE setLaserActive NOTIFY laserActiveChanged)
  Q_PROPERTY(bool yarnActive READ yarnActive WRITE setYarnActive NOTIFY yarnActiveChanged)
  Q_PROPERTY(bool peekabooActive READ peekabooActive WRITE setPeekabooActive NOTIFY peekabooActiveChanged)
  Q_PROPERTY(qreal toyRot READ toyRot NOTIFY toyRotChanged)
  Q_PROPERTY(QString monitor READ monitor NOTIFY changed)
  Q_PROPERTY(QStringList screens READ screens NOTIFY screensChanged)
  Q_PROPERTY(QString feedback READ feedback NOTIFY feedbackChanged)
  Q_PROPERTY(bool outOfTheWay READ outOfTheWay NOTIFY outOfTheWayChanged)
  Q_PROPERTY(bool timeFatigued READ timeFatigued NOTIFY timeFatiguedChanged)
  Q_PROPERTY(int xp READ xp NOTIFY xpChanged)
  Q_PROPERTY(int level READ level NOTIFY levelChanged)
  Q_PROPERTY(qreal levelProgress READ levelProgress NOTIFY xpChanged)
  Q_PROPERTY(int xpInLevel READ xpInLevel NOTIFY xpChanged)
  Q_PROPERTY(int xpForNextLevel READ xpForNextLevel CONSTANT)
  Q_PROPERTY(QString levelTitle READ levelTitle NOTIFY levelChanged)
  Q_PROPERTY(QString nextUnlock READ nextUnlock NOTIFY levelChanged)
  Q_PROPERTY(bool testing READ testing CONSTANT)

public:
  Backend(QString configPath, bool preview, bool testing, Mascot *mascot,
          Theme *theme, Cursor *cursor, Activity *activity,
          Compositor *compositor, Music *music, Keyboard *keyboard = nullptr,
          QObject *parent = nullptr);

  qreal size() const { return m_size; }
  QString colorMode() const { return m_colorMode; }
  QColor mascotColor() const;
  QColor mascotColor2() const;
  QString customColor() const { return m_customColor; }
  QString gradientColor() const { return m_gradientColor; }
  QString furPattern() const { return m_furPattern; }
  qreal furType() const;
  QString dogFurSource() const { return m_dogFurSource; }
  bool followCursor() const { return m_followCursor; }
  bool idleAntics() const { return m_idleAntics; }
  bool reactToDesktop() const { return m_reactToDesktop; }
  bool sleepWhenIdle() const { return m_sleepWhenIdle; }
  bool reducedMotion() const { return m_reducedMotion; }
  bool stayOnTop() const { return m_stayOnTop; }
  bool startAtLogin() const;
  bool showTrayIcon() const { return m_showTrayIcon; }
  QString language() const { return m_language; }
  bool mochiPetting() const { return m_mochiPetting; }
  bool mochiBubbles() const { return m_mochiBubbles; }
  int mochiInterval() const { return m_mochiInterval; }
  QString keyboardReaction() const { return m_keyboardReaction; }
  bool keyboardAvailable() const;
  qreal keyPulse() const { return m_keyPulse; }
  QString monitor() const { return m_monitor; }
  QStringList screens() const;
  QString feedback() const { return m_feedback; }
  QString accessory() const { return m_accessory; }
  void setAccessory(const QString &acc);
  bool musicPlaying() const;
  bool laserActive() const { return m_laserActive; }
  void setLaserActive(bool active);
  bool yarnActive() const { return m_yarnActive; }
  void setYarnActive(bool active);
  bool peekabooActive() const { return m_peekabooActive; }
  void setPeekabooActive(bool active);
  qreal toyRot() const { return m_toyRot; }
  // True while she has stepped aside, which is to say while something is
  // fullscreen. QML fades her out rather than hiding the window, because a
  // layer surface cannot be given its role back once it has been destroyed.
  bool outOfTheWay() const { return m_outOfTheWay; }
  bool testing() const { return m_testing; }

  qreal nx() const { return m_place.x(); }
  qreal ny() const { return m_place.y(); }
  bool timeFatigued() const { return m_timeFatigued; }

  int xp() const { return m_xp; }
  int level() const { return 1 + m_xp / 100; }
  qreal levelProgress() const { return (m_xp % 100) / 100.0; }
  int xpInLevel() const { return m_xp % 100; }
  int xpForNextLevel() const { return 100; }
  QString levelTitle() const;
  QString nextUnlock() const;
  QString xpStatus() const;

  void attach(QQuickWindow *window);
  void attachBubble(QQuickWindow *bubble);
  void repositionBubble();
  void attachSnackWindow(QQuickWindow *snack);
  void repositionSnackWindow();
  void attachToyWindow(QQuickWindow *toy);
  void repositionToyWindow();
  Q_INVOKABLE void toyClicked();
  Q_INVOKABLE void showSnackMenu();
  Q_INVOKABLE void toggleSnackMenu();

  Q_INVOKABLE void configure(const QString &key, const QVariant &value);
  Q_INVOKABLE void setStartAtLogin(bool enabled);
  Q_INVOKABLE void resetPlace();
  Q_INVOKABLE void openSettings();
  Q_INVOKABLE void quit();
  Q_INVOKABLE void command(const QString &name);
  Q_INVOKABLE void pet();
  Q_INVOKABLE void love();
  Q_INVOKABLE void feed(const QString &snack = QStringLiteral("cookie"));
  Q_INVOKABLE void showSpeech(const QString &text = QString());
  Q_INVOKABLE void showRandomThought();
  Q_INVOKABLE void toggleLaser();
  Q_INVOKABLE void toggleYarn();
  Q_INVOKABLE void triggerPeekaboo();
  Q_INVOKABLE void celebrate();
  Q_INVOKABLE void tickle();
  Q_INVOKABLE void stretch();
  Q_INVOKABLE void sleep();
  Q_INVOKABLE void addXp(int amount, const QString &source = QString());
  Q_INVOKABLE void backflip();
  Q_INVOKABLE void highFive();
  Q_INVOKABLE void starShower();
  Q_INVOKABLE void danceBeat();
  QString status() const;


  // Drag. QML reports movement as a delta in the item's own coordinates,
  // because a Wayland client cannot trust mapToGlobal() for a layer surface --
  // it has no reliable idea where the compositor put it.
  Q_INVOKABLE void grabDrag();
  Q_INVOKABLE void dragBy(qreal dx, qreal dy);
  Q_INVOKABLE void releaseDrag();
  Q_INVOKABLE bool dragging() const { return m_dragging; }

  // Drive flight. Called from the same frame callback as the mascot's tick so
  // the two never drift apart.
  Q_INVOKABLE void advance(qreal dt);
  Q_INVOKABLE void launch(qreal dx, qreal dy, qreal speed);
  bool flying() const { return m_flying; }

  // Run through everything she can do, once, in order. Handy for seeing the
  // whole repertoire without waiting on her own timing.
  Q_INVOKABLE void demo();
  bool demoing() const { return m_demoStep >= 0; }

  // Test seams.
  void injectCursor(int x, int y);
  qreal windowSize() const;
  qreal bodyRadius() const;
  QPointF centreOnScreen() const;

signals:
  void changed();
  void showTrayIconChanged();
  void languageChanged();
  void mochiPettingChanged();
  void mochiBubblesChanged();
  void mochiIntervalChanged();
  void snackRequested(const QString &snack);
  void keyPulseChanged();
  void outOfTheWayChanged();
  void screensChanged();
  void feedbackChanged();
  void settingsRequested();
  void accessoryChanged();
  void musicPlayingChanged();
  void laserActiveChanged();
  void yarnActiveChanged();
  void peekabooActiveChanged();
  void peekabooTriggered();
  void celebrateTriggered();
  void timeFatiguedChanged();
  void toyRotChanged();
  void xpChanged();
  void levelChanged();
  void leveledUp(int newLevel, const QString &rewardTitle, const QString &rewardDesc);

private:
  void load();
  void save();
  void applyToMascot();
  void applyPlacement();
  void applyInputRegion(bool wholeWindow = false);
  void remapToLiveScreen();
  void note(const QString &message);
  QRect screenGeometry() const;
  void onKeyPressed();

  QString m_configPath;
  bool m_preview = false;
  bool m_testing = false;
  Mascot *m_mascot = nullptr;
  Theme *m_theme = nullptr;
  Cursor *m_cursor = nullptr;
  Activity *m_activity = nullptr;
  Compositor *m_compositor = nullptr;
  Music *m_music = nullptr;
  Keyboard *m_keyboard = nullptr;
  bool m_outOfTheWay = false;
  qreal m_sinceBusyThought = 0.0;
  QQuickWindow *m_window = nullptr;
  bool m_layered = false;
  QQuickWindow *m_bubbleWindow = nullptr;
  bool m_bubbleLayered = false;
  QQuickWindow *m_snackWindow = nullptr;
  bool m_snackLayered = false;
  qreal m_sinceThought = 0.0;
  QTimer m_dodgeTimer;
  QTimer m_remapTimer;
  qreal m_dodgeOffset = 0.0;
  qreal m_dodgeOffsetTarget = 0.0;

  qreal m_size = 1.0;
  QString m_language = QStringLiteral("fr");
  QString m_colorMode = QStringLiteral("ink");
  QString m_customColor = QStringLiteral("#a855f7");
  QString m_gradientColor = QStringLiteral("#ec4899");
  QString m_furPattern = QStringLiteral("calico");
  QString m_dogFurSource = QStringLiteral("theme");
  QString m_keyboardReaction = QStringLiteral("bounce");
  qreal m_keyPulse = 0.0;
  bool m_followCursor = true;
  bool m_idleAntics = true;
  bool m_reactToDesktop = true;
  bool m_sleepWhenIdle = true;
  bool m_reducedMotion = false;
  bool m_stayOnTop = true;
  bool m_showTrayIcon = false;
  bool m_mochiPetting = true;
  bool m_mochiBubbles = true;
  int m_mochiInterval = 180;
  QString m_accessory = QStringLiteral("none");
  bool m_laserActive = false;
  bool m_yarnActive = false;
  bool m_peekabooActive = false;
  bool m_timeFatigued = false;
  QString m_monitor;
  QPointF m_place{0.86, 0.74};

  QString m_feedback;
  QTimer m_saveTimer;

  // Drag bookkeeping.
  bool m_dragging = false;
  qreal m_dragSpeed = 0.0;
  QPointF m_dragVelocity;   // pixels per second, smoothed
  QElapsedTimer m_dragClock;

  // Flight, in fractions of the screen's travel per second.
  bool m_flying = false;
  QPointF m_flightVelocity;

  // Demo playback.
  int m_demoStep = -1;
  qreal m_demoClock = 0.0;

  // Desktop toy window & state
  QQuickWindow *m_toyWindow = nullptr;
  bool m_toyLayered = false;
  QPointF m_toyPos;
  QPointF m_toyVel;
  qreal m_toyRot = 0.0;
  qreal m_timeSinceToyPounce = 0.0;
  qreal m_peekabooTimer = 0.0;

  // XP & Evolution state
  int m_xp = 0;
  qreal m_timeSinceLastPetXp = 99.0;
  qreal m_activeTimeXpTimer = 0.0;
};

