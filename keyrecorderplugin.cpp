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


void KeyRecorderView::insertKeyRecorder()
{
  if (recording) {
    recording = false;
//     qApp->removeEventFilter(this);
    m_view->focusProxy()->removeEventFilter(this);

    qDebug() << "-----";

    int prev_key = 0;
    QEvent::Type prev_type = QEvent::None;
    for (auto & kevent : kevents) {
      qDebug() <<  kevent.type << ' ' << kevent.key << ' ' << kevent.modifiers << ' ' << kevent.text;

      // si release mais pas de press alors shortcut

      if (kevent.modifiers && kevent.type == QEvent::KeyRelease
       && (prev_key != kevent.key || prev_type != QEvent::KeyPress)
      ) {
        qDebug() << "QEvent::ShortcutOverride";
        int k = kevent.key;
        if (kevent.modifiers & Qt::META) {
          k += Qt::META;
        }
        if (kevent.modifiers & Qt::CTRL) {
          k += Qt::CTRL;
        }
        if (kevent.modifiers & Qt::ALT) {
          k += Qt::ALT;
        }
        if (kevent.modifiers & Qt::SHIFT) {
          k += Qt::SHIFT;
        }
        QKeySequence keyseq(k);
        for (QAction * action : m_view->actionCollection()->actions()) {
          KAction * kaction = static_cast<KAction*>(action);
          //qDebug() << kaction->text() << ": " <<  kaction->shortcut();
          if (kaction->shortcut().contains(keyseq)) {
            kaction->trigger();
            break;
          }
        }
      }
      else {
        QKeyEvent event(
          kevent.text.isEmpty() ? QEvent::KeyPress : kevent.type
        , kevent.key, kevent.modifiers, kevent.text);
        QApplication::sendEvent(m_view->focusProxy(), &event);
//         QApplication::sendEvent(qApp->focusWidget(), &event);
      }

      prev_key = kevent.key;
      prev_type = kevent.type;
    }
    kevents.clear();

//     {
//       QKeyEvent event(
//         QEvent::KeyRelease, 86
//       , Qt::KeyboardModifiers(0x4000000));
//       QApplication::sendEvent(m_view->focusProxy(), &event);
//     }
//     {
//       QKeyEvent event(
//         QEvent::ShortcutOverride, 86
//       , Qt::KeyboardModifiers(0x4000000));
// //       QApplication::sendEvent(m_view->focusProxy(), &event);
//       qDebug() << QApplication::sendEvent(m_view->focusProxy(), &event);
//
//       for (QAction * action : m_view->actionCollection()->actions()) {
//         KAction * kaction = static_cast<KAction*>(action);
//         qDebug() << kaction->text() << ": " <<  kaction->shortcut();
//         if (kaction->shortcut().contains(QKeySequence(Qt::CTRL + Qt::Key_H))) {
//           kaction->trigger();
//         }
//       }
//     }
//     {
//       QKeyEvent event(QEvent::KeyRelease, 86, Qt::KeyboardModifiers(0x4000000));
// //       QShortcutEvent event(Qt::CTRL + Qt::Key_B, 1);
// //       QKeyEvent event(QEvent::KeyPress, 61, Qt::KeyboardModifiers(), "a");
// //       QApplication::sendEvent(m_view->focusProxy(), &event);
//       qDebug() << QApplication::sendEvent(m_view->focusProxy(), &event);
// //       qDebug() << QApplication::sendEvent(m_view, &event);
// //       qDebug() << QApplication::sendEvent(m_view->document(), &event);
// //       qDebug() << QApplication::sendEvent(m_view->actionCollection(), &event);
// //       qDebug() << QApplication::sendEvent(m_view->focusWidget(), &event);
// // //       qDebug() << QApplication::sendEvent(m_view->keyboardGrabber(), &event);
// //       qDebug() << QApplication::sendEvent(m_view->window(), &event);
// //       qDebug() << QApplication::sendEvent(m_view->parentWidget(), &event);
//     }
//     {
//       QKeyEvent event(QEvent::KeyRelease, 61, Qt::KeyboardModifiers(), "a");
// //       QApplication::sendEvent(m_view->focusProxy(), &event);
//       QApplication::sendEvent(m_view->focusProxy(), &event);
//     }
// //     {
// //       QKeyEvent event(
// //         QEvent::KeyRelease, 86
// //       , Qt::KeyboardModifiers(0x4000000));
// //       QApplication::sendEvent(m_view->focusProxy(), &event);
// //     }

    qDebug() << "clear";
  }
  else {
    recording = true;
//     qApp->installEventFilter(this);
    m_view->focusProxy()->installEventFilter(this);
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
  if (obj != m_view->focusProxy()) {
    return false;
  }

  if ((event->type() == QEvent::KeyPress
    || event->type() == QEvent::KeyRelease
   //|| event->type() == QEvent::ShortcutOverride
   // vimode double event filter
   ) && event->spontaneous()
  ) {
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
        event->type()
      , kevent->key()
      , kevent->modifiers()
      , kevent->text()
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
