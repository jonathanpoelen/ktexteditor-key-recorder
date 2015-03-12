#ifndef KEYRECORDERVIEW_H
#define KEYRECORDERVIEW_H

#include <QList>
#include <QObject>
#include <QKeyEvent>
#include <KXMLGUIClient>

class KeyRecorderView : public QObject, public KXMLGUIClient
{
	Q_OBJECT
public:
  explicit KeyRecorderView(KTextEditor::View *view = 0);
  ~KeyRecorderView();

private slots:
  void insertKeyRecorder();

private:
  bool eventFilter(QObject* obj, QEvent* event);

  KTextEditor::View *m_view;

  struct Key {
    QEvent::Type type;
    int key;
    Qt::KeyboardModifiers modifiers;
    QString text;
  };

  QList<Key> kevents;
  bool recording = false;
  QObject * event_obj = nullptr;
};

#endif
