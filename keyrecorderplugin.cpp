#include "keyrecorderplugin.h"
#include "keyrecorderview.h"

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
// #include <ktexteditor/messageinterface.h>

#include <KPluginFactory>
#include <KActionCollection>
#include <KAction>
#include <kaboutdata.h>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QMenu>

K_PLUGIN_FACTORY(KeyRecorderPluginFactory, registerPlugin<KeyRecorderPlugin>("ktexteditor_keyrecorder");)
K_EXPORT_PLUGIN(KeyRecorderPluginFactory(KAboutData(
  "ktexteditor_keyrecorder",
  "ktexteditor_plugins",
  ki18n("KeyRecorder"),
  "0.1",
  ki18n("Record a key sequence"),
  KAboutData::License_LGPL_V3,
  ki18n("Author: Jonathan Poelen"),
  KLocalizedString(),
  "https://github.com/jonathanpoelen/ktexteditor-key-recorder",
  "jonathan.poelen+KeyRecorder@gmail.com"
)))

KeyRecorderPlugin::KeyRecorderPlugin(QObject *parent, const QVariantList &)
: KTextEditor::Plugin(parent)
{
}

KeyRecorderPlugin::~KeyRecorderPlugin()
{
}

void KeyRecorderPlugin::addView(KTextEditor::View *view)
{
  KeyRecorderView *nview = new KeyRecorderView(this, view);
  m_views.append(nview);
  if (m_mode == Mode::recording) {
    nview->activateRecording();
    m_event_obj->removeEventFilter(this);
    m_event_obj = view->focusProxy();
    m_event_obj->installEventFilter(this);
  }
}

void KeyRecorderPlugin::removeView(KTextEditor::View *view)
{
	for(int z = 0; z < m_views.size(); z++)
	{
		if(m_views.at(z)->parentClient() == view)
		{
			KeyRecorderView *nview = m_views.at(z);
			m_views.removeAll(nview);
			delete nview;
		}
	}
}

void KeyRecorderPlugin::readConfig()
{
}

void KeyRecorderPlugin::writeConfig()
{
}

void KeyRecorderPlugin::actionTriggered(QAction* a)
{
  //qDebug() << "triggered";
  // TODO What to do if "A" is destroyed? Search by name?
  m_kevents.append(Key{
    a
  , {} /*a->objectName()*/
  , QEvent::Type()
  , 0
  , 0
  });
}

struct KeyRecorderPlugin::Lock {
  Lock(KeyRecorderPlugin * k, Mode new_mode)
  : m(k->m_mode)
  { m = new_mode; }

  ~Lock()
  { m = Mode::wait; }

private:
  Mode & m;
};

void KeyRecorderPlugin::replay()
{
  if (m_mode != Mode::wait) {
    return ;
  }

  Lock lock(this, Mode::replay);

  for (auto & kevent : m_kevents) {
    //qDebug() << kevent.text;
    if (kevent.action) {
      kevent.action->trigger();
    }
    else {
      QKeyEvent event(
        kevent.text.isEmpty() ? QEvent::KeyPress : kevent.type
      , kevent.key, kevent.modifiers, kevent.text);
      QApplication::sendEvent(qApp->focusWidget(), &event);
    }
  }
}

void KeyRecorderPlugin::recording(KTextEditor::View * view)
{
  if (m_mode == Mode::recording) {
    for (KeyRecorderView * v : m_views) {
      v->deactivateRecording();
    }
    if (m_event_obj) {
      m_event_obj->removeEventFilter(this);
      m_event_obj = nullptr;
    }
    m_mode = Mode::wait;
    //replay_action->setDisabled(false);
  }
  else if (m_mode == Mode::wait) {
    m_kevents.clear();
    m_mode = Mode::recording;
    m_in_context_menu = false;
    m_event_obj = view->focusProxy();
    m_event_obj->installEventFilter(this);
    for (KeyRecorderView * v : m_views) {
      v->activateRecording();
    }

//     KTextEditor::MessageInterface *iface
//       = qobject_cast<KTextEditor::MessageInterface*>( m_view->document() );
//
//     if (iface) {
//       KTextEditor::Message * message = new KTextEditor::Message(
//         recordingText()
//       , KTextEditor::Message::Information);
//       message.setText();
//   //     message->setView(m_view);
//       message->setWordWrap(true);
//   //     message->setAutoHide();
//       iface->postMessage(message);
//     }
  }
}

bool KeyRecorderPlugin::eventFilter(QObject* obj, QEvent* event)
{
  QEvent::Type const type = event->type();

  if (type == QEvent::FocusOut) {
    auto * focus_widget = qApp->focusWidget();
    if (focus_widget) {
      //qDebug() << "focus: " << focus_widget;
      m_event_obj->removeEventFilter(this);
      m_event_obj = focus_widget;
      m_event_obj->installEventFilter(this);
      m_in_context_menu = qobject_cast<QMenu*>(focus_widget);
    }
  }
  else if (!m_in_context_menu && (
      type == QEvent::KeyPress
   || type == QEvent::KeyRelease
   //|| type == QEvent::ShortcutOverride
   // vimode double event filter
   ) && event->spontaneous()
  ) {
    const auto kevent = static_cast<QKeyEvent*>(event);

    //qDebug() << event;

    switch (kevent->key()) {
      //BEGIN modifiers
      case 16777248:
      case 16777249:
      case 16777250:
      case 16777251:
      case 16781571:
        break;
      //END
      default:
      m_kevents.append({
        nullptr
      , kevent->text()
      , event->type()
      , kevent->modifiers()
      , kevent->key()
      });
    }
    return false;
  }

  return QObject::eventFilter(obj, event);
}


static QString startRecordingText() {
  static QString const s = i18n("Start Record Keystroke");
  return s;
};

static QString stopRecordingText() {
  static QString const s = i18n("Stop Record Keystroke");
  return s;
};


KeyRecorderView::KeyRecorderView(KeyRecorderPlugin * plugin, KTextEditor::View *view)
: QObject(view)
, KXMLGUIClient(view)
, m_plugin(plugin)
, m_view(view)
{
	setComponentData(KeyRecorderPluginFactory::componentData());

	KAction *action;

  action = m_recording_action = new KAction(startRecordingText(), this);
  actionCollection()->addAction("tools_keyrecorder_recorder", action);
	//action->setShortcut(Qt::CTRL + Qt::Key_XYZ);
	connect(action, SIGNAL(triggered()), this, SLOT(recording()));

  action = /*m_replay_action = */new KAction(i18n("Replay keystroke"), this);
  //action->setDisabled(true);
	actionCollection()->addAction("tools_keyrecorder_replay", action);
  connect(action, SIGNAL(triggered()), m_plugin, SLOT(replay()));

	setXMLFile("keyrecorderui.rc");
}

KeyRecorderView::~KeyRecorderView()
{
  m_info_recording->deleteLater();
}

void KeyRecorderView::recording()
{
  m_plugin->recording(m_view);
}

void KeyRecorderView::activateRecording()
{
  if (QLayout * layout = m_view->layout()) {
    if (!m_info_recording) {
      m_info_recording = new QLabel(i18n("<b>Record</b> keystroke"));
    }
    m_info_recording->setParent(m_view);
    layout->addWidget(m_info_recording);
  }
  connect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
  , m_plugin, SLOT(actionTriggered(QAction*)));
  m_recording_action->setText(stopRecordingText());
}

void KeyRecorderView::deactivateRecording()
{
  if (QLayout * layout = m_view->layout()) {
    layout->removeWidget(m_info_recording);
  }
  m_info_recording->setParent(nullptr);
  disconnect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
  , m_plugin, SLOT(actionTriggered(QAction*)));
  m_recording_action->setText(startRecordingText());
}


// // editor is of type KTextEditor::Editor*
// KTextEditor::CommandInterface *iface =
// qobject_cast<KTextEditor::CommandInterface*>( editor );
// if( iface ) {
// // the implementation supports the interface
// // do stuff
// }

#include "keyrecorderview.moc"
#include "keyrecorderplugin.moc"
