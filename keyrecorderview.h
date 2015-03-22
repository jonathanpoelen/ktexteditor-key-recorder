#ifndef KEYRECORDERVIEW_H
#define KEYRECORDERVIEW_H

#include <QList>
#include <QObject>
#include <QKeyEvent>
#include <KXMLGUIClient>

class QAction;

class KeyRecorderView : public QObject, public KXMLGUIClient
{
	Q_OBJECT
public:
  explicit KeyRecorderView(KTextEditor::View *view = 0);
  ~KeyRecorderView();

private slots:
  void insertKeyRecorder();
  void actionTriggered(QAction*);

private:
  bool eventFilter(QObject* obj, QEvent* event);

  KTextEditor::View *m_view;

  struct Key {
    QAction * action;
    QString text;
    QEvent::Type type;
    Qt::KeyboardModifiers modifiers;
    int key;
  };

  QList<Key> kevents;
  QObject * event_obj = nullptr;
  bool recording = false;
  bool in_context_menu = false;
};

#endif
