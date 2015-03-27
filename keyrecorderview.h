#ifndef KEYRECORDERVIEW_H
#define KEYRECORDERVIEW_H

#include <QList>
#include <QObject>
#include <QKeyEvent>
#include <KXMLGUIClient>

namespace KTextEditor {
  class View;
}

class KeyRecorderPlugin;
class KAction;
class QLabel;

class KeyRecorderView : public QObject, public KXMLGUIClient
{
	Q_OBJECT
public:
  explicit KeyRecorderView(KeyRecorderPlugin * plugin, KTextEditor::View * view);
  ~KeyRecorderView();

private slots:
  void recording1();
  void recording2();
  void recording3();
  void replay1();
  void replay2();
  void replay3();

private:
  void activateRecording(unsigned);
  void deactivateRecording(unsigned);

  KeyRecorderPlugin * m_plugin;
  KTextEditor::View * m_view;
  struct Widget {
    KAction * recording;
    KAction * replay;
  } m_widgets[3];
  QLabel * m_info;
  KAction * m_replay_action;
  KAction * m_stop_action;

  friend KeyRecorderPlugin;
};

#endif
