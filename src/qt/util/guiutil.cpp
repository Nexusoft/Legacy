/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "guiutil.h"
#include "addressvalidator.h"
#include "../models/walletmodel.h"
#include "../core/units.h"

#include <QString>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
 #include <QUrlQuery>
 #include <QStandardPaths>
#endif
#include <QTextDocument> // For Qt::escape
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>

namespace GUIUtil {

QString dateTimeStr(const QDateTime &date)
{
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
}

QString dateTimeStr(qint64 nTime)
{
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
}

QFont NexusAddressFont()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

void setupAddressWidget(QLineEdit *widget, QWidget *parent)
{
    widget->setMaxLength(NexusAddressValidator::MaxAddressLength);
    widget->setValidator(new NexusAddressValidator(parent));
    widget->setFont(NexusAddressFont());
}

void setupAmountWidget(QLineEdit *widget, QWidget *parent)
{
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

bool parseNexusURI(const QUrl &uri, SendCoinsRecipient *out)
{
    if(uri.scheme() != QString("Nexus"))
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
	#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
	 QList<QPair<QString, QString> > items = uri.queryItems();
	#else
	 QUrlQuery query(uri);
     QList<QPair<QString, QString> > items = query.queryItems();
	#endif
    for (QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
        bool fShouldReturnFalse = false;
        if (i->first.startsWith("req-"))
        {
            i->first.remove(0, 4);
            fShouldReturnFalse = true;
        }

        if (i->first == "label")
        {
            rv.label = i->second;
            fShouldReturnFalse = false;
        }
        else if (i->first == "amount")
        {
            if(!i->second.isEmpty())
            {
                if(!NexusUnits::parse(NexusUnits::Nexus, i->second, &rv.amount))
                {
                    return false;
                }
            }
            fShouldReturnFalse = false;
        }

        if (fShouldReturnFalse)
            return false;
    }
    if(out)
    {
        *out = rv;
    }
    return true;
}

bool parseNexusURI(QString uri, SendCoinsRecipient *out)
{
    // Convert Nexus:// to Nexus:
    //
    //    Cannot handle this later, because Nexus:// will cause Qt to see the part after // as host,
    //    which will lowercase it (and thus invalidate the address).
    if(uri.startsWith("Nexus://"))
    {
        uri.replace(0, 9, "Nexus:");
    }
    QUrl uriInstance(uri);
    return parseNexusURI(uriInstance, out);
}

QString HtmlEscape(const QString& str, bool fMultiLine)
{
	#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
	 QString escaped = Qt::escape(str);
	#else
	 QString escaped = QString(str).toHtmlEscaped();
	#endif
    if(fMultiLine)
    {
        escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
}

QString HtmlEscape(const std::string& str, bool fMultiLine)
{
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
}

void copyEntryData(QAbstractItemView *view, int column, int role)
{
    if(!view || !view->selectionModel())
        return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
        // Copy first item
        QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
}

QString getSaveFileName(QWidget *parent, const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 QString *selectedSuffixOut)
{
    QString selectedFilter;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
        #if QT_VERSION < QT_VERSION_CHECK(5,0,0)
		 myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
		#else
		 myDir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
		#endif
    }
    else
    {
        myDir = dir;
    }
	
	QFileDialog dialog;
	dialog.setStyleSheet("background-color: #F7F7F7; selection-background-color: #084B8A;");
    QString result = dialog.getSaveFileName(parent, caption, myDir, filter, &selectedFilter, QFileDialog::DontUseNativeDialog);

    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(selectedFilter))
    {
        selectedSuffix = filter_re.cap(1);
    }

    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
        if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
        {
            /* No suffix specified, add selected suffix */
            if(!result.endsWith("."))
                result.append(".");
            result.append(selectedSuffix);
        }
    }

    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
        *selectedSuffixOut = selectedSuffix;
    }
    return result;
}

Qt::ConnectionType blockingGUIThreadConnection()
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        return Qt::BlockingQueuedConnection;
    }
    else
    {
        return Qt::DirectConnection;
    }
}

bool checkPoint(const QPoint &p, const QWidget *w)
{
  QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
  if(!atW) return false;
  return atW->topLevelWidget() == w;
}

bool isObscured(QWidget *w)
{

  return !(checkPoint(QPoint(0, 0), w)
           && checkPoint(QPoint(w->width() - 1, 0), w)
           && checkPoint(QPoint(0, w->height() - 1), w)
           && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
           && checkPoint(QPoint(w->width()/2, w->height()/2), w));
}

} // namespace GUIUtil

