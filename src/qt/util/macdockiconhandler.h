/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#ifndef MACDOCKICONHANDLER_H
#define MACDOCKICONHANDLER_H

#include <QtCore/QObject>

class QMenu;
class QIcon;
class QWidget;
class objc_object;

/** Macintosh-specific dock icon handler.
 */
class MacDockIconHandler : public QObject
{
    Q_OBJECT
public:
    ~MacDockIconHandler();

    QMenu *dockMenu();
    void setIcon(const QIcon &icon);

    static MacDockIconHandler *instance();

    void handleDockIconClickEvent();

signals:
    void dockIconClicked();

public slots:

private:
    MacDockIconHandler();

    objc_object *m_dockIconClickEventHandler;
    QWidget *m_dummyWidget;
    QMenu *m_dockMenu;
};

#endif // MACDOCKICONCLICKHANDLER_H
