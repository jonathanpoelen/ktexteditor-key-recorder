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
  void recording();

private:
  void activateRecording();
  void deactivateRecording();

  KeyRecorderPlugin * m_plugin;
  KTextEditor::View * m_view;
  KAction * m_recording_action;
  QLabel * m_info_recording = nullptr;

  friend KeyRecorderPlugin;
};

#endif
