#include "variantmaptablemodel.h"

VariantMapTableModel::VariantMapTableModel(QObject *parent) : QAbstractTableModel (parent)
{

}

void VariantMapTableModel::registerColumn(AbstractColumn *column)
{
    // todo: проверки на повторяемость и тд
    _colums.append(column);
}

void VariantMapTableModel::addRow(QVariantMap rowData)
{
    // "id" можно вынести в отдельным параметром
    int id = rowData.value("id").toInt();
    beginInsertRows(QModelIndex(), _rowIndex.count(), _rowIndex.count());
    _rowIndex.append(id);
    _dataHash.insert(id, rowData);
    endInsertRows();
}

int VariantMapTableModel::idByRow(int row) const
{
    return _rowIndex.at(row);
}

int VariantMapTableModel::colByName(QString name) const
{
    for (int col = 0; col < _colums.count(); ++col) {
        if (nameByCol(col) == name)
            return col;
    }
    return -1;
}

QString VariantMapTableModel::nameByCol(int col) const
{
    return _colums.at(col)->name();
}

int VariantMapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _rowIndex.count();
}

int VariantMapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _colums.count();
}

QVariant VariantMapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int id = idByRow(index.row());
    QVariantMap rowData = _dataHash.value(id);
    return _colums.at(index.column())->colData(rowData, role);
}

bool VariantMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == Qt::EditRole) {
        int id = idByRow(index.row());
        _dataHash[id].insert(nameByCol(index.column()), value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags VariantMapTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> VariantMapTableModel::roleNames() const
{
    auto r = QAbstractTableModel::roleNames();
    for (int i = 0; i < _colums.size(); ++i) {
        r.insert(Qt::UserRole + i, _colums.at(i)->name().toUtf8());
    }
    return r;
}

SimpleColumn::SimpleColumn(QString name) : AbstractColumn (name)
{

}

QVariant SimpleColumn::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    return rowData.value(name());
}

AbstractColumn::AbstractColumn(QString name) : _name(name)
{

}

FullnameColumn::FullnameColumn(QString name) : AbstractColumn (name)
{

}

QVariant FullnameColumn::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    return rowData.value("firstname").toString() + " " + rowData.value("lastname").toString();
}
