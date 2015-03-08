#ifndef KEYRECORDERPLUGIN_H
#define KEYRECORDERPLUGIN_H

#include <KTextEditor/Plugin>

namespace KTextEditor
{
	class View;
}

class KeyRecorderView;

class KeyRecorderPlugin
  : public KTextEditor::Plugin
{
  public:
    // Constructor
    explicit KeyRecorderPlugin(QObject *parent = 0, const QVariantList &args = QVariantList());
    // Destructor
    virtual ~KeyRecorderPlugin();

    void addView (KTextEditor::View *view);
    void removeView (KTextEditor::View *view);
 
    void readConfig();
    void writeConfig();
 
//     void readConfig (KConfig *);
//     void writeConfig (KConfig *);
 
  private:
    QList<class KeyRecorderView*> m_views;
};

#endif
