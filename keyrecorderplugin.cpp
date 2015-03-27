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
#include <QtGlobal>

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


template<class> struct array_size;
template<class T, unsigned N> struct array_size<T[N]>
{ static unsigned const value = N; };

template<class List, unsigned N>
static
#ifdef IN_IDE_PARSER
QList< KeyRecorderPlugin::Event> &
#else
List &
#endif
last_events(List (&akevents)[N])
{ return akevents[N-1]; }


KeyRecorderPlugin::KeyRecorderPlugin(QObject *parent, const QVariantList &)
: KTextEditor::Plugin(parent)
, m_record_idx(0)
, m_in_context_menu(false)
{
  static_assert(
      array_size<decltype(KeyRecorderView::m_widgets)>::value
    ==
      array_size<decltype(m_events_list)>::value - 1
  , "difference in the size of the arrays");
}

KeyRecorderPlugin::~KeyRecorderPlugin()
{
}

void KeyRecorderPlugin::addView(KTextEditor::View *view)
{
  KeyRecorderView *nview = new KeyRecorderView(this, view);
  m_views.append(nview);

  bool all_is_empty = true;
  for (unsigned i = 0; i < array_size<decltype(m_events_list)>::value-1; ++i) {
    if (m_events_list[i].empty()) {
      nview->m_widgets[i].replay->setDisabled(true);
    }
    else {
      all_is_empty = false;
    }
  }
  if (all_is_empty) {
    nview->m_replay_action->setDisabled(true);
  }

  if (m_mode == Mode::recording) {
    nview->activateRecording(m_record_idx);
    nview->m_stop_action->setDisabled(false);
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

struct KeyRecorderPlugin::Lock {
  Lock(KeyRecorderPlugin * k, Mode new_mode)
  : m(k->m_mode)
  , old(m)
  { m = new_mode; }

  ~Lock()
  { m = old; }

private:
  Mode & m;
  Mode old;
};

void KeyRecorderPlugin::replay(unsigned record_idx)
{
  if (m_mode == Mode::replay) {
    return ;
  }

  if (m_mode == Mode::recording) {
    last_events(m_events_list).append(m_events_list[record_idx]);
  }

  m_record_idx = record_idx;

  Lock lock(this, Mode::replay);

  for (auto & kevent : m_events_list[record_idx]) {
    qDebug() << kevent.text;
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

void KeyRecorderPlugin::replay()
{
  replay(m_record_idx);
}

void KeyRecorderPlugin::recording(KTextEditor::View * view, unsigned record_idx)
{
  if (m_mode == Mode::recording) {
    m_events_list[record_idx].clear();
    m_events_list[record_idx].swap(last_events(m_events_list));
    bool const is_empty = m_events_list[record_idx].empty();
    for (KeyRecorderView * v : m_views) {
      v->deactivateRecording(record_idx);
      if (!is_empty) {
        v->m_widgets[record_idx].replay->setDisabled(false);
      }
      v->m_replay_action->setDisabled(false);
    }
    if (m_event_obj) {
      m_event_obj->removeEventFilter(this);
      m_event_obj = nullptr;
    }
    m_mode = Mode::wait;
    //replay_action->setDisabled(false);
  }
  else if (m_mode == Mode::wait) {
    m_mode = Mode::recording;
    m_record_idx = record_idx;
    m_in_context_menu = false;
    m_event_obj = view->focusProxy();
    m_event_obj->installEventFilter(this);
    for (KeyRecorderView * v : m_views) {
      v->activateRecording(record_idx);
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

void KeyRecorderPlugin::stop()
{
  if (m_mode == Mode::recording) {
    recording(nullptr, m_record_idx);
  }
}

void KeyRecorderPlugin::actionTriggered(QAction* a)
{
  if (m_mode != Mode::recording) {
    return;
  }
  qDebug() << "triggered";
  // TODO What to do if "a" is destroyed? Search by name?
  last_events(m_events_list).append(Event{
    a
  , {} /*a->objectName()*/
  , QEvent::Type()
  , 0
  , 0
  });
}

bool KeyRecorderPlugin::eventFilter(QObject* obj, QEvent* event)
{
  if (m_mode != Mode::recording) {
    return QObject::eventFilter(obj, event);
  }

  QEvent::Type const type = event->type();

  if (type == QEvent::FocusOut) {
    auto * focus_widget = qApp->focusWidget();
    if (focus_widget) {
      qDebug() << "focus: " << focus_widget;
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

    qDebug() << event <<  "  " << kevent->key();

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
      last_events(m_events_list).append({
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


static QString startRecordingText(unsigned i) {
  static QString const s[]{
    i18n("Start Record Keystroke 1")
  , i18n("Start Record Keystroke 2")
  , i18n("Start Record Keystroke 3")
  };
  return s[i];
};

static QString stopRecordingText(unsigned i) {
  static QString const s[]{
    i18n("Stop Record Keystroke 1")
  , i18n("Stop Record Keystroke 2")
  , i18n("Stop Record Keystroke 3")
  };
  return s[i];
};


KeyRecorderView::KeyRecorderView(KeyRecorderPlugin * plugin, KTextEditor::View *view)
: QObject(view)
, KXMLGUIClient(view)
, m_plugin(plugin)
, m_view(view)
, m_info(nullptr)
{
	setComponentData(KeyRecorderPluginFactory::componentData());

#define INIT(i) {\
  auto & w = m_widgets[i - 1];\
  w.recording = new KAction(startRecordingText(i-1), this);\
  actionCollection()->addAction("tools_keyrecorder_recorder" #i, w.recording);\
  connect(w.recording, SIGNAL(triggered()), this, SLOT(recording ## i()));\
  w.replay = new KAction(i18n("Replay keystroke" #i), this);\
  actionCollection()->addAction("tools_keyrecorder_replay" #i, w.replay);\
  connect(w.replay, SIGNAL(triggered()), this, SLOT(replay ## i()));\
}
  INIT(1);
  INIT(2);
  INIT(3);

#undef INIT

  KAction * a = m_replay_action = new KAction(i18n("Replay keystroke"), this);
  actionCollection()->addAction("tools_keyrecorder_replay", a);
  connect(a, SIGNAL(triggered()), m_plugin, SLOT(replay()));

  a = m_stop_action = new KAction(i18n("Stop Record Keystroke"), this);
  a->setDisabled(true);
  actionCollection()->addAction("tools_keyrecorder_stop", a);
  connect(a, SIGNAL(triggered()), m_plugin, SLOT(stop()));

	setXMLFile("keyrecorderui.rc");
}

KeyRecorderView::~KeyRecorderView()
{
  if (m_info) {
    m_info->deleteLater();
  }
}

void KeyRecorderView::recording1() { m_plugin->recording(m_view, 0); }
void KeyRecorderView::recording2() { m_plugin->recording(m_view, 1); }
void KeyRecorderView::recording3() { m_plugin->recording(m_view, 2); }

void KeyRecorderView::replay1() { m_plugin->replay(0); }
void KeyRecorderView::replay2() { m_plugin->replay(1); }
void KeyRecorderView::replay3() { m_plugin->replay(2); }

void KeyRecorderView::activateRecording(unsigned record_idx)
{
  if (QLayout * layout = m_view->layout()) {
    if (!m_info) {
      m_info = new QLabel(i18n("<b>Record</b> keystroke"));
    }
    m_info->setParent(m_view);
    layout->addWidget(m_info);
  }
  connect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
  , m_plugin, SLOT(actionTriggered(QAction*)));
  for (unsigned i = 0; i < array_size<decltype(m_widgets)>::value; ++i) {
    if (i == record_idx) {
      m_widgets[i].recording->setText(stopRecordingText(i));
    }
    else {
      m_widgets[i].recording->setDisabled(true);
    }
  }
  m_stop_action->setDisabled(false);
}

void KeyRecorderView::deactivateRecording(unsigned record_idx)
{
  if (QLayout * layout = m_view->layout()) {
    layout->removeWidget(m_info);
  }
  m_widgets[record_idx].recording->setParent(nullptr);
  disconnect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
  , m_plugin, SLOT(actionTriggered(QAction*)));
  for (unsigned i = 0; i < array_size<decltype(m_widgets)>::value; ++i) {
    if (i == record_idx) {
      m_widgets[i].recording->setText(startRecordingText(i));
    }
    else {
      m_widgets[i].recording->setDisabled(false);
    }
  }
  m_stop_action->setDisabled(true);
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
