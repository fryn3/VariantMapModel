#ifndef VARIANTMAPMODEL_H
#define VARIANTMAPMODEL_H

#include <QAbstractTableModel>

class AbstractColumnRole
{
public:
    AbstractColumnRole(QString name);
    virtual ~AbstractColumnRole() = default;
    QString name() { return _name; }
    virtual QVariant colData(const QVariantMap &rowData, int role = Qt::DisplayRole) = 0;
private:
    QString _name;
};

using AbstractColumn = AbstractColumnRole;
using AbstractRole = AbstractColumnRole;

class SimpleColumn : public AbstractColumnRole
{
public:
    SimpleColumn(QString name);

    QVariant colData(const QVariantMap &rowData, int role) override;
};

class FullnameColumn : public AbstractColumnRole
{
public:
    FullnameColumn(QString name);

    QVariant colData(const QVariantMap &rowData, int role) override;
};

class VariantMapModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static const QString MODULE_NAME;   // "VariantMap"
    static const bool IS_QML_REG;

    VariantMapModel(QObject *parent = nullptr);
    VariantMapModel(bool isList, bool autoId = false, bool withHeading = false, QObject *parent = nullptr);
    void registerColumn(AbstractColumn *column);
    void registerRole(AbstractRole *role);
    void addRow(QVariantMap rowData);
    void removeId(int id);
    void removeRow(int row);
    void removeAllRows();

    QVariantMap getRowData(int row) const;
    int idByRow(int row) const;
    int colByName(QString name) const;
    QString nameByCol(int col) const;
    bool getForListViewFormat() const;
    void setForListViewFormat(bool forListViewFormat);
    bool autoId() const;
    void setAutoId(bool autoId);
    bool getWithHeading() const;
    void setWithHeading(bool value);
    QString getIdStr() const;
    void setIdStr(const QString &id);
    int calcRow(const QModelIndex &index) const;
    bool isHeadingRow(const QModelIndex &index) const;
    QJsonValue toJson() const;
    QCborValue toCbor() const;
    QByteArray toByteArray(bool isJson) const;
    void fromJson(QJsonValue jValue);
    void fromCbor(QCborValue cborValue);
    void fromByteArray(QByteArray buff, bool isJson);
private:
    QList<int> _rowIndex;
    QHash<int, QVariantMap> _dataHash;
    QList<AbstractColumn*> _columns;
    QList<AbstractRole*> _roles;
    mutable QHash<int, QByteArray> _rolesId;
    QString _idStr = "id";  ///< строковое представление ключа
    uint _idRow = 0;
    bool _forListViewFormat = false;
    bool _autoId = false;
    bool _withHeading = false;

public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // VARIANTMAPMODEL_H
