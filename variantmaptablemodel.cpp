#include "variantmaptablemodel.h"

uint qHash( const QVariant &var, uint seed)
{
    if ( !var.isValid() || var.isNull() ) {
        Q_ASSERT(false);
        return -1;
    }

    switch ( var.type() )
    {
    case QVariant::Int:
        return qHash( var.toInt(), seed);
    case QVariant::UInt:
        return qHash( var.toUInt(), seed);
    case QVariant::Bool:
        return qHash( var.toUInt(), seed);
    case QVariant::Double:
        return qHash( var.toUInt(), seed);
    case QVariant::LongLong:
        return qHash( var.toLongLong(), seed);
    case QVariant::ULongLong:
        return qHash( var.toULongLong(), seed);
    case QVariant::String:
        return qHash( var.toString(), seed);
    case QVariant::Char:
        return qHash( var.toChar(), seed);
    case QVariant::StringList:
        return qHash( var.toString(), seed);
    case QVariant::ByteArray:
        return qHash( var.toByteArray(), seed);
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::RegExp:
        return qHash( var.toString(), seed);
    case QVariant::Map:
    case QVariant::List:
    case QVariant::BitArray:
    case QVariant::Size:
    case QVariant::SizeF:
    case QVariant::Rect:
    case QVariant::LineF:
    case QVariant::Line:
    case QVariant::RectF:
    case QVariant::Point:
    case QVariant::PointF:
        // not supported yet
        Q_ASSERT(false);
        return -1;
    case QVariant::UserType:
    case QVariant::Invalid:
    default:
        Q_ASSERT(false);
        return -1;
    }
    Q_ASSERT(false);
    // could not generate a hash for the given variant
    return -1;
}

uint qHash(const QVariantMap &var, uint seed)
{
    uint hash = 0;
    auto it = var.cbegin();
    while (it != var.cend()) {
        hash ^= qHash(it.key(), seed) ^ qHash(it.value(), seed);
    }
    return hash;
}

VariantMapTableModel::VariantMapTableModel(QObject *parent) : QAbstractTableModel (parent)
{

}

bool VariantMapTableModel::registerColumn(AbstractColumn *column)
{
    for (const auto& c: _colums) {
        if (c->name() == column->name()) {
            return false;
        }
    }
    _colums.append(column);
    return true;
}

uint VariantMapTableModel::qHashRow(QVariantMap rowData)
{
    // либо оставить только id либо использовать qHash (тогда строчки не могут повтаряться
    return qHash(rowData) ^ _id++;
}

void VariantMapTableModel::appendRow(QVariantMap rowData)
{
    uint hash = qHashRow(rowData);
    beginInsertRows(QModelIndex(), _rowIndex.count(), _rowIndex.count());
    _rowIndex.append(hash);
    _dataHash.insert(hash, rowData);
    endInsertRows();
}

uint VariantMapTableModel::idByRow(int row) const
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
    if (role >= Qt::UserRole) {
        return data(this->index(index.row(), role - Qt::UserRole), Qt::DisplayRole);
    }
    uint id = idByRow(index.row());
    QVariantMap rowData = _dataHash.value(id);
    return _colums.at(index.column())->colData(rowData, role);
}

bool VariantMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == Qt::EditRole) {
        uint id = idByRow(index.row());
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
    auto roles = QAbstractTableModel::roleNames();
    for (int col = 0; col < _colums.count(); ++col) {
        roles.insert(Qt::UserRole + col, _colums.at(col)->name().toUtf8());
    }
    return roles;
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
