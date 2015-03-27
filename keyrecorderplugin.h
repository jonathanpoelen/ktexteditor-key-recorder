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

  void addView(KTextEditor::View *view);
  void removeView(KTextEditor::View *view);

  void readConfig();
  void writeConfig();

  //void readConfig (KConfig *);
  //void writeConfig (KConfig *);

public:
  void recording(KTextEditor::View * view, unsigned record_idx);

public slots:
  void replay(unsigned record_idx);
  void replay();
  void stop();

private slots:
  void actionTriggered(QAction*);

private:
  bool eventFilter(QObject* obj, QEvent* event);

  QList<KeyRecorderView*> m_views;

  struct Event {
    QAction * action;
    QString text;
    QEvent::Type type;
    Qt::KeyboardModifiers modifiers;
    int key;
  };

  enum class Mode { wait, recording, replay, };

  QList<Event> m_events_list[4];
  QObject * m_event_obj = nullptr;
  Mode m_mode = Mode::wait;
  unsigned m_record_idx;
  bool m_in_context_menu;

  class Lock;
};

#endif
