#include "keyrecorderplugin.h"
#include "keyrecorderview.h"

#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <ktexteditor/messageinterface.h>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocale>
#include <KAction>
#include <KActionCollection>
#include <kaboutdata.h>
#include <kapplication.h>
#include <QApplication>
#include <QDesktopWidget>
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

KeyRecorderPlugin::KeyRecorderPlugin(QObject *parent, const QVariantList &args)
: KTextEditor::Plugin(parent)
{
	Q_UNUSED(args);
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

KeyRecorderView::KeyRecorderView(KTextEditor::View *view)
: QObject(view)
, KXMLGUIClient(view)
, m_view(view)
{
	setComponentData(KeyRecorderPluginFactory::componentData());

	KAction *action = new KAction(i18n("KTextEditor - KeyRecorder"), this);
	actionCollection()->addAction("tools_keyrecorder", action);
	//action->setShortcut(Qt::CTRL + Qt::Key_XYZ);
	connect(action, SIGNAL(triggered()), this, SLOT(insertKeyRecorder()));

	setXMLFile("keyrecorderui.rc");
}

KeyRecorderView::~KeyRecorderView()
{
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

void KeyRecorderView::insertKeyRecorder()
{
  if (recording) {
    if (!event_obj) {
      qDebug() << "is locked";
      return ;
    }
//     m_view->focusProxy()->removeEventFilter(this);

    disconnect(m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
    , this, SLOT(actionTriggered(QAction*)));
    event_obj->removeEventFilter(this);
    event_obj = nullptr;

    qDebug() << "-----";
    qDebug() << "focusWidget " << qApp->focusWidget();

    // NOTE removes the last keyboard
//     if (!kevents.empty()) {
//       kevents.pop_back();
//     }

    for (auto & kevent : kevents) {
      qDebug() <<  kevent.type << ' ' << kevent.key << ' ' << kevent.modifiers << ' ' << kevent.text << ' ' << kevent.action;

      /// TODO not event if focus is a KMenu(/QMenu) (add contexte in kevent)

      // si release mais pas de press alors shortcut

      if (kevent.action) {
        kevent.action->trigger();
      }
      else {
        QKeyEvent event(
          kevent.text.isEmpty() ? QEvent::KeyPress : kevent.type
        , kevent.key, kevent.modifiers, kevent.text);
        qDebug() << "focusWidget: " << qApp->focusWidget();
        QApplication::sendEvent(qApp->focusWidget(), &event);
      }

//       qDebug() << "position: " << m_view->cursorPosition();
    }
    kevents.clear();

    qDebug() << "clear";
    recording = false;
  }
  else {
    recording = true;
    in_context_menu = false;
    event_obj = m_view->focusProxy();
    event_obj->installEventFilter(this);

    connect(
      m_view->actionCollection(), SIGNAL(actionTriggered(QAction*))
    , this, SLOT(actionTriggered(QAction*)));

//     qApp->installEventFilter(this);
//     m_view->focusProxy()->installEventFilter(this);
//     KTextEditor::Message * message = new KTextEditor::Message("start record");
// //     message->setView(m_view);
//     message->setWordWrap(true);
// //     message->setAutoHide();
//     KTextEditor::MessageInterface *iface =
//         qobject_cast<KTextEditor::MessageInterface*>( m_view->document() );
//     qDebug() << "iface: " << iface;
//
//     if( !iface ) {
//         // the implementation does not support the interface
//     }
//     iface->postMessage(message);
  }
	//m_view->document()->insertText(m_view->cursorPosition(), i18n("Hello, World!"));
}

bool KeyRecorderView::eventFilter(QObject* obj, QEvent* event)
{
//   if (obj != m_view->focusProxy()) {
//     if (obj != qApp->focusWidget()) {
//       return false;
//     }
//   }

  QEvent::Type const type = event->type();

  if (type == QEvent::ShortcutOverride) {
    qDebug() << "ShortcutOverride ";
  }
  if (type == QEvent::FocusOut) {
    auto focus_widget = qApp->focusWidget();
    if (focus_widget) {
      event_obj->removeEventFilter(this);
      event_obj = focus_widget;
      event_obj->installEventFilter(this);
      in_context_menu = qobject_cast<QMenu*>(focus_widget);
    }
    qDebug() << "focusOut " << obj << ' ' << focus_widget;
  }
  if (type == QEvent::FocusIn) {
    qDebug() << "focusIn " << obj;
  }

  if (!in_context_menu && (type == QEvent::KeyPress
    || type == QEvent::KeyRelease
   //|| type == QEvent::ShortcutOverride
   // vimode double event filter
   ) && event->spontaneous()
  ) {
    qDebug() << m_view->focusProxy() << ' ' << qApp->focusWidget() << ' ' << obj;
    const auto kevent = static_cast<QKeyEvent*>(event);

    switch (kevent->key()) {
      case 16777248:
      case 16777249:
      case 16777250:
      case 16777251:
      case 16781571:
        break;
      default:
      kevents.append({
        nullptr
      , kevent->text()
      , event->type()
      , kevent->modifiers()
      , kevent->key()
      });
    }
    using P = void*;
    qDebug() << obj << P(event) << ' ' << event;
    return false;
  }
//   qDebug() << "type: " << event->type();
  return QObject::eventFilter(obj, event);
}

#include "keyrecorderview.moc"
