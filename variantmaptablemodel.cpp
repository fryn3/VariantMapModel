#include "variantmaptablemodel.h"

#include "QQmlEngine"
#include <QDebug>

bool registerMe()
{
    qmlRegisterType<VariantMapTableModel>(VariantMapTableModel::MODULE_NAME.toUtf8(), 1, 0, "VariantMapTableModel");
    return true;
}

const QString VariantMapTableModel::MODULE_NAME = "VariantMapTable";

const bool VariantMapTableModel::IS_QML_REG = registerMe();


VariantMapTableModel::VariantMapTableModel(QObject *parent) : QAbstractTableModel (parent)
{

}

void VariantMapTableModel::registerColumn(AbstractColumn *column)
{
    // todo: проверки на повторяемость и тд
    _columns.append(column);
}

void VariantMapTableModel::registerRole(AbstractRole *role)
{
    // todo: проверки на повторяемость и тд
    _roles.append(role);
}

void VariantMapTableModel::addRow(QVariantMap rowData)
{
    // "id" можно вынести в отдельный параметр
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
    qDebug() << __PRETTY_FUNCTION__ << "вроде не нужна";
    for (int col = 0; col < _columns.count(); ++col) {
        if (nameByCol(col) == name)
            return col;
    }
    return -1;
}

QString VariantMapTableModel::nameByCol(int col) const
{
    return _columns.at(col)->name();
}

bool VariantMapTableModel::getWithHeading() const
{
    return _withHeading;
}

void VariantMapTableModel::setWithHeading(bool value)
{
    _withHeading = value;
}

int VariantMapTableModel::calcRow(const QModelIndex &index) const
{
    return index.row() - _withHeading;
}

bool VariantMapTableModel::getForListViewFormat() const
{
    return _forListViewFormat;
}

void VariantMapTableModel::setForListViewFormat(bool forListViewFormat)
{
    _forListViewFormat = forListViewFormat;
}

int VariantMapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _rowIndex.count() + _withHeading;
}

int VariantMapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _columns.count();
}

QVariant VariantMapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role > Qt::UserRole && _forListViewFormat) {
        return data(this->index(calcRow(index), role - Qt::UserRole), Qt::DisplayRole);
    }
    if (calcRow(index) < 0) {
        if (role == Qt::DisplayRole) {
            return _columns.at(index.column())->name();
        } else {
            return QVariant();
        }
    }
    int id = idByRow(calcRow(index));
    QVariantMap rowData = _dataHash.value(id);
    if (role == Qt::DisplayRole) {
        return _columns.at(index.column())->colData(rowData, role);
    } else {
        qDebug() << rowData[_rolesId[role]];
        return rowData[_rolesId[role]];
    }
}

bool VariantMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || calcRow(index) < 0) {
        return false;
    }
    if (role == Qt::EditRole) {
        int id = idByRow(calcRow(index));
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
    _rolesId = QAbstractTableModel::roleNames();
    for (int i = 0; i < _columns.size(); ++i) {
        _rolesId.insert(Qt::UserRole + i, _columns.at(i)->name().toUtf8());
    }
    for (int i = 0; i < _roles.size(); ++i) {
        _rolesId.insert(Qt::UserRole + _columns.size() + i, _roles.at(i)->name().toUtf8());
    }
    return _rolesId;
}

SimpleColumn::SimpleColumn(QString name) : AbstractColumnRole (name)
{

}

QVariant SimpleColumn::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    return rowData.value(name());
}

AbstractColumnRole::AbstractColumnRole(QString name) : _name(name)
{

}

FullnameColumn::FullnameColumn(QString name) : AbstractColumnRole (name)
{

}

QVariant FullnameColumn::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    return rowData.value("firstname").toString() + " " + rowData.value("lastname").toString();
}
