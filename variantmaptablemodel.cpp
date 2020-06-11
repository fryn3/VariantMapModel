#include "variantmaptablemodel.h"

#include "QQmlEngine"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

bool registerMe()
{
    qmlRegisterType<VariantMapTableModel>(VariantMapTableModel::MODULE_NAME.toUtf8(), 1, 0, "VariantMapTableModel");
    return true;
}

const QString VariantMapTableModel::MODULE_NAME = "VariantMapTable";

const bool VariantMapTableModel::IS_QML_REG = registerMe();


VariantMapTableModel::VariantMapTableModel(QObject *parent)
    : QAbstractTableModel (parent) { }

VariantMapTableModel::VariantMapTableModel(bool isList, bool autoId, bool withHeading, QObject *parent)
    : QAbstractTableModel (parent), _forListViewFormat(isList), _autoId(autoId),
      _withHeading(withHeading) { }

void VariantMapTableModel::registerColumn(AbstractColumn *column)
{
    // todo: проверки на повторяемость и тд
    _columns.append(column);
}

void VariantMapTableModel::registerRole(AbstractRole *role)
{
    // todo: можно избавиться от этой ф-ции, а добавлять все лишние роли при addRow
    // todo: проверки на повторяемость и тд
    _roles.append(role);
}

void VariantMapTableModel::addRow(QVariantMap rowData)
{
    int id = _autoId ? ++_idRow : rowData.value(_idStr).toInt();
    beginInsertRows(QModelIndex(), _rowIndex.count(), _rowIndex.count());
    _rowIndex.append(id);
    _dataHash.insert(id, rowData);
    endInsertRows();
}

QVariantMap VariantMapTableModel::getRowData(int row) const
{
    int id = idByRow(row);
    return _dataHash.value(id);
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

bool VariantMapTableModel::isHeadingRow(const QModelIndex &index) const
{
    return calcRow(index) < 0;
}

QByteArray VariantMapTableModel::toJson(bool isBin) const
{
    QJsonArray jArr;
    for (int row = 0; row < _rowIndex.count(); ++row) {
        auto rowData = getRowData(row);
        QJsonObject jRow = QJsonObject::fromVariantMap(rowData);
        jArr.append(jRow);
    }
    if (isBin)
        return QCborValue::fromJsonValue(jArr).toCbor();
    return QJsonDocument(jArr).toJson();
}

void VariantMapTableModel::fromJson(QByteArray buff, bool isBin)
{
    QJsonArray jArr;
    if (isBin) {
        QCborValue cbor = QCborValue::fromCbor(buff);
        jArr = cbor.toJsonValue().toArray();
    } else {
        jArr = QJsonDocument::fromJson(buff).array();
    }
    for (const auto& jRowRef: jArr) {
        QVariantMap item = jRowRef.toObject().toVariantMap();
        addRow(item);
    }
}

bool VariantMapTableModel::autoId() const
{
    return _autoId;
}

void VariantMapTableModel::setAutoId(bool autoId)
{
    _autoId = autoId;
}

QString VariantMapTableModel::getIdStr() const
{
    return _idStr;
}

void VariantMapTableModel::setIdStr(const QString &id)
{
    _idStr = id;
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
    if (isHeadingRow(index)) {
        if (role == Qt::DisplayRole) {
            return _columns.at(index.column())->name();
        } else {
            return QVariant();
        }
    }
    QVariantMap rowData = getRowData(calcRow(index));
    if (role == Qt::DisplayRole) {
        return _columns.at(index.column())->colData(rowData, role);
    } else {
        qDebug() << rowData[_rolesId[role]];
        return rowData[_rolesId[role]];
    }
}

bool VariantMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || isHeadingRow(index)) {
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
