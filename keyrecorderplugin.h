#ifndef KEYRECORDERPLUGIN_H
#define KEYRECORDERPLUGIN_H

#include <KTextEditor/Plugin>
#include <QKeyEvent>

namespace KTextEditor
{
	class View;
}

class KeyRecorderView;
class QAction;

class KeyRecorderPlugin
: public KTextEditor::Plugin
{
  Q_OBJECT
public:
  // Constructor
  explicit KeyRecorderPlugin(QObject *parent = 0, const QVariantList &args = QVariantList());
  // Destructor
  virtual ~KeyRecorderPlugin();

  void addView (KTextEditor::View *view);
  void removeView (KTextEditor::View *view);

  void readConfig();
  void writeConfig();

  //void readConfig (KConfig *);
  //void writeConfig (KConfig *);

public:
  void recording(KTextEditor::View * view);

public slots:
  void replay();

private slots:
  void actionTriggered(QAction*);

private:
  bool eventFilter(QObject* obj, QEvent* event);

  QList<KeyRecorderView*> m_views;

  struct Key {
    QAction * action;
    QString text;
    QEvent::Type type;
    Qt::KeyboardModifiers modifiers;
    int key;
  };

  enum class Mode { wait, recording, replay, };

  QList<Key> m_kevents;
  QObject * m_event_obj = nullptr;
  Mode m_mode = Mode::wait;
  bool m_in_context_menu = false;

  class Lock;
};

#endif
