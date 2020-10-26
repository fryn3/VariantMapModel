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

/*!
 * \brief Универсальная модель
 *
 * Модель содержит данные в формате QVariant.
 * Может выдавать данные как в режиме Table, так и в режиме List. Для
 * управления выдачи данных используйте setListViewFormat.
 * Все данные должны иметь числовой идентификатор. Если идентификатора в данных
 * нет, нужно установить autoId в true. Для установки строкового представления
 * ключа, используйте setIdStr(). По умолчанию установлено "id".
 * Можно регистрировать колонки и роли. В режиме Table столбцы будут
 * выводится, в отличии от ролей. Роли распространяются на всю строку.
 */
class VariantMapModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static const QString MODULE_NAME;   // "VariantMap"
    static const QString ITEM_NAME;     // "VariantMapModel"
    static const bool IS_QML_REG;

    VariantMapModel(QObject *parent = nullptr);

    /*!
     * \brief Конструктор с параметрами
     * \param isList - если true, то работа идет в стиле List, иначе в
     * стиле Table.
     * \param autoId - автогенерация числового идентификатора. Если false, то
     * необходимо передавать.
     * идентификатор по ключу getIdStr().
     * \param parent - родитель в стиле Qt.
     */
    VariantMapModel(bool isList, bool autoId = false, QObject *parent = nullptr);

    /*!
     * \brief Добавления колонки
     *
     * Отличие колонок от ролей то, что колонки предназначены для вывода на
     * экран. Порядок регистрации колонок важен в режиме Table.
     * \param column - задает имя колонки.
     * \todo Реализовать удаление колонок и возможно перемещение.
     * \todo Можно возвращать номер столбца.
     */
    void registerColumn(AbstractColumnRole *column);

    /*!
     * \brief Добавления роли
     * \param role - задает имя роли.
     */
    void registerRole(AbstractColumnRole *role);

    /*!
     * \brief Добавление строк
     * \param rowData - содержит данные колоное и ролей.
     */
    virtual bool addRow(const QVariantMap &rowData);

    /*!
     * \brief Удаляет строку со с идентификатором id
     * \param id - идентификатор строчки для удаления.
     * \return true - если строка удалилась, иначе false.
     */
    bool removeId(int id);

    /*!
     * \brief Удаляет все строчки(данные) модели
     * \return true - если успешно.
     */
    Q_INVOKABLE bool removeAllRows();

    /*!
     * \brief Получает данные строчки
     * \param row - номер строки в модели.
     * \return Данные строчки.
     */
    QVariantMap getRowData(int row) const;

    /*!
     * \brief Получает идентификатор по номеру строки
     * \param row - номер строки.
     * \return идентификатор.
     */
    int idByRow(int row) const;

    /*!
     * \brief Возвращает номер колонки по имени
     * \param name - строковое представление.
     * \return номер колонки.
     */
    int colByName(QString name) const;

    /*!
     * \brief Возвращает имя по номеру колонки
     * \param col - номер колонки.
     * \return имя колонки.
     */
    QString nameByCol(int col) const;

    /*!
     * \brief Возвращает режим работы
     * \return true, если установлен формат List, иначе Table.
     */
    bool getListViewFormat() const;

    /*!
     * \brief Установка формата выдачи
     * \param listViewFormat - true если List, иначе Table.
     */
    void setListViewFormat(bool listViewFormat);

    /*!
     * \brief Возвращает состояние автогенерации идентификатора
     * \return true, если автогенерация включена.
     */
    bool autoId() const;

    /*!
     * \brief Установка настройки автогенерации индикатора
     * \param autoId - true если необходимо включить автогенерацию.
     */
    void setAutoId(bool autoId);

    /*!
     * \brief Получение имени идентификатора
     * \return Строковое представление идентификатора.
     */
    QString getIdStr() const;

    /*!
     * \brief Установка идентификатора
     * \param id - строковое представление идентификатора.
     */
    void setIdStr(const QString &id);

    /*!
     * \brief Перевести данные модели в формат JSON
     * \return Данные модели в формате JSON.
     */
    QJsonValue toJson() const;

    /*!
     * \brief Перевести данные модели в формат CBOR
     * \return Данные модели в формате CBOR.
     */
    QCborValue toCbor() const;

    /*!
     * \brief Переводит данные в массив байт
     * \param isJson - true, тогда в формате JSON, если false, тогда в
     * формате CBOR.
     * \return массив байт.
     */
    QByteArray toByteArray(bool isJson) const;

    /*!
     * \brief Получает данные из JSON формата
     * \param jValue - данные модели.
     */
    void fromJson(QJsonValue jValue);

    /*!
     * \brief Получает данные из CBOR формата
     * \param cborValue - данные модели.
     */
    void fromCbor(QCborValue cborValue);

    /*!
     * \brief Получает данные из массива байт
     * \param buff - данные модели
     * \param isJson - true, если данные в формате JSON, иначе в
     * формате CBOR.
     */
    void fromByteArray(QByteArray buff, bool isJson);

    /*!
     * \brief Возвращает Md5 хэш данных
     * \return Массив байт.
     */
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
