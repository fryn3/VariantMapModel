#ifndef VARIANTMAPTABLEMODEL_H
#define VARIANTMAPTABLEMODEL_H

#include <QAbstractTableModel>

uint qHash(const QVariant &var, uint seed = 1);

uint qHash(const QVariantMap &var, uint seed = 1);

class AbstractColumn
{
public:
    AbstractColumn(QString name);
    virtual ~AbstractColumn() = default;
    QString name() { return _name; }
    virtual QVariant colData(const QVariantMap &rowData, int role = Qt::DisplayRole) = 0;
private:
    QString _name;
};

class SimpleColumn : public AbstractColumn
{
public:
    SimpleColumn(QString name);

    QVariant colData(const QVariantMap &rowData, int role) override;
};

class FullnameColumn : public AbstractColumn
{
public:
    FullnameColumn(QString name);

    QVariant colData(const QVariantMap &rowData, int role) override;
};

class VariantMapTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    VariantMapTableModel(QObject *parent = nullptr);
    bool registerColumn(AbstractColumn *column);
    void appendRow(QVariantMap rowData);

    uint idByRow(int row) const;
    int colByName(QString name) const;
    QString nameByCol(int col) const;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
protected:
    uint qHashRow(QVariantMap rowData);
    // Storing data members
    QList<uint> _rowIndex;
    QHash<uint, QVariantMap> _dataHash;
    QList<AbstractColumn*> _colums;
    uint _id = 0;

};

#endif // VARIANTMAPTABLEMODEL_H
