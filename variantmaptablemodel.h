#ifndef VARIANTMAPTABLEMODEL_H
#define VARIANTMAPTABLEMODEL_H

#include <QAbstractTableModel>

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
    void registerColumn(AbstractColumn *column);
    void addRow(QVariantMap rowData);

    // Convenience methods
    int idByRow(int row) const;
    int colByName(QString name) const;
    QString nameByCol(int col) const;
private:
    // Storing data members
    QList<int> _rowIndex;
    QHash<int, QVariantMap> _dataHash;
    QList<AbstractColumn*> _colums;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // VARIANTMAPTABLEMODEL_H
