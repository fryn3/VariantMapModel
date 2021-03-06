#ifndef VARIANTMAPMODEL_H
#define VARIANTMAPMODEL_H

#include <QAbstractTableModel>

class AbstractColumnRole
{
public:
    AbstractColumnRole(QString name);
    virtual ~AbstractColumnRole() = default;
    QString name() { return _name; }
    virtual QVariant colData(const QVariantMap &rowData, int role = Qt::DisplayRole);
private:
    QString _name;
};

using SimpleColumn = AbstractColumnRole;
using SimpleRole = AbstractColumnRole;

class VariantMapModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static const QString MODULE_NAME;   // "VariantMap"
    static const QString ITEM_NAME;     // "VariantMapModel"
    static const bool IS_QML_REG;

    VariantMapModel(QObject *parent = nullptr);
    VariantMapModel(bool isList, bool autoId = false, bool withHeading = false, QObject *parent = nullptr);
    void registerColumn(AbstractColumnRole *column);
    void registerRole(AbstractColumnRole *role);
    virtual void addRow(const QVariantMap &rowData);
    void removeId(int id);
    Q_INVOKABLE void removeAllRows();

    QVariantMap getRowData(int row) const;
    int idByRow(int row) const;
    int colByName(QString name) const;
    QString nameByCol(int col) const;
    bool getListViewFormat() const;
    void setListViewFormat(bool listViewFormat);
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
    QByteArray hash() const;
protected:
    QString roleStr(int role) const;
    int roleInt(QString role) const;
private:
    QList<int> _rowIndex;
    QHash<int, QVariantMap> _dataHash;
    QList<AbstractColumnRole*> _columns;
    QList<AbstractColumnRole*> _roles;
    QString _idStr = "id";  ///< строковое представление ключа
    uint _idRow = 0;
    bool _listViewFormat = false;
    bool _autoId = false;
    bool _withHeading = false;
    mutable QHash<int, QByteArray> _rolesId;

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
};

#endif // VARIANTMAPMODEL_H
