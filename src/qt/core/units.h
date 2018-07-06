/*******************************************************************************************

            Hash(BEGIN(Satoshi[2010]), END(W.J.[2012])) == Videlicet[2014] ++

 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php

*******************************************************************************************/

#ifndef NexusUNITS_H
#define NexusUNITS_H

#include <QString>
#include <QAbstractListModel>

/** Nexus unit definitions. Encapsulates parsing and formatting
   and serves as list model for dropdown selection boxes.
*/
class NexusUnits: public QAbstractListModel
{
public:
    explicit NexusUnits(QObject *parent);

    /** Nexus units.
      @note Source: https://en.Nexus.it/wiki/Units . Please add only sensible ones
     */
    enum Unit
    {
        Nexus,
        mNexus,
        uNexus
    };

    //! @name Static API
    //! Unit conversion and formatting
    ///@{

    //! Get list of units, for dropdown box
    static QList<Unit> availableUnits();
    //! Is unit ID valid?
    static bool valid(int unit);
    //! Short name
    static QString name(int unit);
    //! Longer description
    static QString description(int unit);
    //! Number of Satoshis (1e-8) per unit
    static qint64 factor(int unit);
    //! Number of amount digits (to represent max number of coins)
    static int amountDigits(int unit);
    //! Number of decimals left
    static int decimals(int unit);
    //! Format as string
    static QString format(int unit, qint64 amount, bool plussign=false);
    //! Format as string (with unit)
    static QString formatWithUnit(int unit, qint64 amount, bool plussign=false);
    //! Parse string to coin amount
    static bool parse(int unit, const QString &value, qint64 *val_out);
    ///@}

    //! @name AbstractListModel implementation
    //! List model for unit dropdown selection box.
    ///@{
    enum RoleIndex {
        /** Unit identifier */
        UnitRole = Qt::UserRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    ///@}
private:
    QList<NexusUnits::Unit> unitlist;
};
typedef NexusUnits::Unit NexusUnit;

#endif // NexusUNITS_H
