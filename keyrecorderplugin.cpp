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
	KeyRecorderView *nview = new KeyRecorderView(view);
	m_views.append(nview);
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



static QString startRecordingText() {
  return i18n("Start record keystroke");
};
static QString stopRecordingText() {
  return i18n("Stop record keystroke");
};


KeyRecorderView::KeyRecorderView(KTextEditor::View *view)
: QObject(view)
, KXMLGUIClient(view)
, m_view(view)
{
	setComponentData(KeyRecorderPluginFactory::componentData());

	KAction *action;

  action = recording_action = new KAction(startRecordingText(), this);
  actionCollection()->addAction("tools_keyrecorder_recorder", action);
	//action->setShortcut(Qt::CTRL + Qt::Key_XYZ);
	connect(action, SIGNAL(triggered()), this, SLOT(recording()));

  action = replay_action = new KAction(i18n("Replay keystroke"), this);
  action->setDisabled(true);
	actionCollection()->addAction("tools_keyrecorder_replay", action);
  connect(action, SIGNAL(triggered()), this, SLOT(replay()));

	setXMLFile("keyrecorderui.rc");
}

KeyRecorderView::~KeyRecorderView()
{
  if (info_recording) {
    info_recording->deleteLater();
  }
}


// // editor is of type KTextEditor::Editor*
// KTextEditor::CommandInterface *iface =
// qobject_cast<KTextEditor::CommandInterface*>( editor );
// if( iface ) {
// // the implementation supports the interface
// // do stuff
// }


void KeyRecorderView::actionTriggered(QAction* a)
{
  // TODO What to do if "A" is destroyed? Search by name?
  kevents.append(KeyRecorderView::Key{
    a
  , {} /*a->objectName()*/
  , QEvent::Type()
  , 0
  , 0
  });
}

struct KeyRecorderView::Lock {
  Lock(KeyRecorderView * k, Mode new_mode)
  : m(k->mode)
  { m = new_mode; }

  ~Lock()
  { m = Mode::wait; }

private:
  Mode & m;
};

void KeyRecorderView::replay()
{
  if (mode != Mode::wait) {
    return ;
  }

  Lock lock(this, Mode::replay);

  for (auto & kevent : kevents) {
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

void KeyRecorderView::recording()
{
  if (mode == Mode::recording) {
    disconnect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
    , this, SLOT(actionTriggered(QAction*)));
    event_obj->removeEventFilter(this);
    event_obj = nullptr;
    mode = Mode::wait;
    recording_action->setText(startRecordingText());
    replay_action->setDisabled(false);

    if (QLayout * layout = m_view->layout()) {
      layout->removeWidget(info_recording);
      info_recording->setParent(nullptr);
    }
  }
  else if (mode == Mode::wait) {
    mode = Mode::recording;
    in_context_menu = false;
    event_obj = m_view->focusProxy();
    event_obj->installEventFilter(this);
    connect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
    , this, SLOT(actionTriggered(QAction*)));
    recording_action->setText(stopRecordingText());

    if (QLayout * layout = m_view->layout()) {
      if (!info_recording) {
        info_recording = new QLabel(i18n("<b>Record</b> keystroke"));
      }
      info_recording->setParent(m_view);
      layout->addWidget(info_recording);
    }

//     KTextEditor::MessageInterface *iface
//       = qobject_cast<KTextEditor::MessageInterface*>( m_view->document() );
//
//     if (iface) {
//       KTextEditor::Message * message = new KTextEditor::Message(
//         startRecordingText()
//       , KTextEditor::Message::Information);
//       message.setText();
//   //     message->setView(m_view);
//       message->setWordWrap(true);
//   //     message->setAutoHide();
//       iface->postMessage(message);
//     }
  }
}

bool KeyRecorderView::eventFilter(QObject* obj, QEvent* event)
{
  QEvent::Type const type = event->type();

  if (type == QEvent::FocusOut) {
    auto * focus_widget = qApp->focusWidget();
    if (focus_widget) {
      event_obj->removeEventFilter(this);
      event_obj = focus_widget;
      event_obj->installEventFilter(this);
      in_context_menu = qobject_cast<QMenu*>(focus_widget);
    }
  }
  else if (!in_context_menu && (
      type == QEvent::KeyPress
   || type == QEvent::KeyRelease
   //|| type == QEvent::ShortcutOverride
   // vimode double event filter
   ) && event->spontaneous()
  ) {
    const auto kevent = static_cast<QKeyEvent*>(event);

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
      kevents.append({
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

#include "keyrecorderview.moc"
