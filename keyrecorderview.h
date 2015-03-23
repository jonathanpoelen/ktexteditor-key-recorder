#ifndef KEYRECORDERVIEW_H
#define KEYRECORDERVIEW_H

#include <QList>
#include <QObject>
#include <QKeyEvent>
#include <KXMLGUIClient>

class QAction;
class QLabel;
class KAction;

namespace KTextEditor {
  class Message;
}

class KeyRecorderView : public QObject, public KXMLGUIClient
{
	Q_OBJECT
public:
  explicit KeyRecorderView(KTextEditor::View *view = 0);
  ~KeyRecorderView();

public slots:
  void recording();
  void replay();

private slots:
  void actionTriggered(QAction*);

private:
  bool eventFilter(QObject* obj, QEvent* event);

  struct Key {
    QAction * action;
    QString text;
    QEvent::Type type;
    Qt::KeyboardModifiers modifiers;
    int key;
  };

  enum class Mode { wait, recording, replay, };

  KTextEditor::View *m_view;
  QList<Key> kevents;
  QObject * event_obj = nullptr;
  KAction * recording_action;
  KAction * replay_action;
  QLabel * info_recording = nullptr;
  Mode mode = Mode::wait;
  bool in_context_menu = false;

  class Lock;
};

#endif
