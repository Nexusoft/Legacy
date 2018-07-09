/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef MONITOREDDATAMAPPER_H
#define MONITOREDDATAMAPPER_H

#include <QtGlobal>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
 #include <QDataWidgetMapper>
#else
 #include <QtWidgets/QDataWidgetMapper>
#endif

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

/** Data to Widget mapper that watches for edits and notifies listeners when a field is edited.
   This can be used, for example, to enable a commit/apply button in a configuration dialog.
 */
class MonitoredDataMapper : public QDataWidgetMapper
{
    Q_OBJECT
public:
    explicit MonitoredDataMapper(QObject *parent=0);

    void addMapping(QWidget *widget, int section);
    void addMapping(QWidget *widget, int section, const QByteArray &propertyName);
private:
    void addChangeMonitor(QWidget *widget);

signals:
    void viewModified();

};



#endif // MONITOREDDATAMAPPER_H
