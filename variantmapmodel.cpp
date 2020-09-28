#include "variantmapmodel.h"

#include <QQmlEngine>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>

static bool registerMe()
{
    qmlRegisterType<VariantMapModel>(VariantMapModel::MODULE_NAME.toUtf8(), 1, 0, VariantMapModel::ITEM_NAME.toUtf8());
    return true;
}

const QString VariantMapModel::MODULE_NAME = "VariantMap";
const QString VariantMapModel::ITEM_NAME = "VariantMapModel";
const bool VariantMapModel::IS_QML_REG = registerMe();


VariantMapModel::VariantMapModel(QObject *parent)
    : QAbstractTableModel (parent) { }

VariantMapModel::VariantMapModel(bool isList, bool autoId, QObject *parent)
    : QAbstractTableModel (parent), _listViewFormat(isList), _autoId(autoId) { }

void VariantMapModel::registerColumn(AbstractColumnRole *column)
{
    // todo: проверки на повторяемость и тд
    _columns.append(column);
}

void VariantMapModel::registerRole(AbstractColumnRole *role)
{
    // todo: можно избавиться от этой ф-ции, а добавлять все лишние роли при addRow
    // todo: проверки на повторяемость и тд
    _roles.append(role);
}

void VariantMapModel::addRow(const QVariantMap &rowData)
{
    int id = _autoId ? ++_idRow : rowData.value(_idStr).toInt();
    beginInsertRows(QModelIndex(), _rowIndex.count(), _rowIndex.count());
    _rowIndex.append(id);
    _dataHash.insert(id, rowData);
    endInsertRows();
    emit dataChanged(index(_rowIndex.count(), 0), index(_rowIndex.count(), columnCount()));
}

void VariantMapModel::removeId(int id)
{
    removeRow(_rowIndex.indexOf(id));
}

void VariantMapModel::removeAllRows()
{
    removeRows(0, _rowIndex.size());
}

QVariantMap VariantMapModel::getRowData(int row) const
{
    int id = idByRow(row);
    return _dataHash.value(id);
}

int VariantMapModel::idByRow(int row) const
{
    return _rowIndex.at(row);
}

int VariantMapModel::colByName(QString name) const
{
    for (int col = 0; col < _columns.count(); ++col) {
        if (nameByCol(col) == name)
            return col;
    }
    return -1;
}

QString VariantMapModel::nameByCol(int col) const
{
    return _columns.at(col)->name();
}

QJsonValue VariantMapModel::toJson() const
{
    QJsonArray jArr;
    for (int row = 0; row < _rowIndex.count(); ++row) {
        auto rowData = getRowData(row);
        QJsonObject jRow = QJsonObject::fromVariantMap(rowData);
        jArr.append(jRow);
    }
    return QJsonValue(jArr);
}

QCborValue VariantMapModel::toCbor() const
{
    return QCborValue::fromJsonValue(toJson());
}

QByteArray VariantMapModel::toByteArray(bool isJson) const
{
    if (isJson) {
        return QJsonDocument(toJson().toObject()).toJson();
    }
    return toCbor().toCbor();
}

void VariantMapModel::fromJson(QJsonValue jValue)
{
    QJsonArray jArr = jValue.toArray();
    for (const auto& jRowRef: jArr) {
        QVariantMap item = jRowRef.toObject().toVariantMap();
        addRow(item);
    }
}

void VariantMapModel::fromCbor(QCborValue cborValue)
{
    QJsonArray jArr = cborValue.toJsonValue().toArray();
    for (const auto& jRowRef: jArr) {
        QVariantMap item = jRowRef.toObject().toVariantMap();
        addRow(item);
    }
}

void VariantMapModel::fromByteArray(QByteArray buff, bool isJson)
{
    if (isJson) {
        fromJson(QJsonDocument::fromJson(buff).array());
    } else {
        fromCbor(QCborValue::fromCbor(buff));
    }
}

QByteArray VariantMapModel::hash() const
{
    return QCryptographicHash::hash(toByteArray(false), QCryptographicHash::Algorithm::Md5);
}

QString VariantMapModel::roleStr(int role) const
{
    return roleNames().value(role);
}

int VariantMapModel::roleInt(QString role) const
{
    QHash<int, QByteArray> roleId = roleNames();
    for (auto it = roleId.cbegin(); it != roleId.cend(); ++it) {
        if (it.value() == role) {
            return it.key();
        }
    }
    return -1;
}

bool VariantMapModel::autoId() const
{
    return _autoId;
}

void VariantMapModel::setAutoId(bool autoId)
{
    _autoId = autoId;
}

QString VariantMapModel::getIdStr() const
{
    return _idStr;
}

void VariantMapModel::setIdStr(const QString &id)
{
    _idStr = id;
}

bool VariantMapModel::getListViewFormat() const
{
    return _listViewFormat;
}

void VariantMapModel::setListViewFormat(bool listViewFormat)
{
    _listViewFormat = listViewFormat;
}

int VariantMapModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _rowIndex.count();
}

int VariantMapModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _columns.count();
}

QVariant VariantMapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) { return QVariant(); }

    if (role >= Qt::UserRole && _listViewFormat) {
        return data(this->index(index.row(), role - Qt::UserRole), Qt::DisplayRole);
    }
    QVariantMap rowData = getRowData(index.row());
    if (role == Qt::DisplayRole) {
        return _columns.at(index.column())->colData(rowData, role);
    } else {
        qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << rowData[_rolesId[role]];
        return rowData[_rolesId[role]];
    }
}

bool VariantMapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) { return false; }

    if (role >= Qt::UserRole) {
        return setData(this->index(index.row(), role - Qt::UserRole), value, Qt::EditRole);
    }
    qDebug() << __PRETTY_FUNCTION__ << index.row() << index.column() << value << role;
    if (role == Qt::EditRole) {
        int id = idByRow(index.row());
        _dataHash[id].insert(nameByCol(index.column()), value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags VariantMapModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> VariantMapModel::roleNames() const
{
    if (!_rolesId.isEmpty()) { return _rolesId; }
//    _rolesId = QAbstractTableModel::roleNames();
    for (int i = 0; i < _columns.size(); ++i) {
        _rolesId.insert(Qt::UserRole + i, _columns.at(i)->name().toUtf8());
    }
    for (int i = 0; i < _roles.size(); ++i) {
        _rolesId.insert(Qt::UserRole + _columns.size() + i, _roles.at(i)->name().toUtf8());
    }
    return _rolesId;
}

bool VariantMapModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!_autoId) {
        return QAbstractTableModel::insertRows(row, count, parent);
    }
    beginInsertRows(parent, row, row + count);
    for (int i = 0; i < count; ++i) {
        int id = ++_idRow;
        _rowIndex.insert(row + i, id);
        _dataHash.insert(id, QVariantMap());
    }
    endInsertRows();
    emit dataChanged(parent, parent);
    return true;
}

bool VariantMapModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    if (row < 0 || row + count > _rowIndex.size()) {
        return false;
    }
    for (int i = 0; i < count; ++i) {
        int id = _rowIndex.takeAt(row);
        _dataHash.remove(id);
    }
    endRemoveRows();
    emit dataChanged(index(row, 0), index(row + count - 1, columnCount(parent)));
    return true;
}

AbstractColumnRole::AbstractColumnRole(QString name) : _name(name) { }

QVariant AbstractColumnRole::colData(const QVariantMap &rowData, int role)
{
    if (role != Qt::DisplayRole) {
        qDebug() << __PRETTY_FUNCTION__;
        return QVariant();
    }
    return rowData.value(name());
}
