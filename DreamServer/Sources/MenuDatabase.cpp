#include "include\MenuDatabase.h"
#include <assert.h>
#include <qsqlquery.h>
#include <qbuffer.h>
#include <qvariant.h>


MenuDatabase::MenuDatabase(QSqlDatabase db)
{
	m_sql_query = new QSqlQuery(db);

	m_sql_query->exec("CREATE TABLE IF NOT EXISTS Menu("
		"Id INT UNIQUE,"
		"Name TEXT,"
		"Category TEXT,"
		"Icon BLOB);");
	assert(m_sql_query->isActive());
}



MenuDatabase::~MenuDatabase()
{
	delete m_sql_query;
}



bool MenuDatabase::add_menu_item(const MenuItem& item)
{
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);
	item.img.save(&buffer, "PNG");

	m_sql_query->prepare("INSERT INTO Menu (Id, Name, Category, Icon)"
		                           "VALUES (?,  ?,    ?,        ?   );");
	m_sql_query->bindValue(0, item.id);
	m_sql_query->bindValue(1, item.name);
	m_sql_query->bindValue(2, item.category);
	m_sql_query->bindValue(3, QVariant(ba));
	m_sql_query->exec();
	return m_sql_query->isActive();
}



bool MenuDatabase::remove_menu_item(int id)
{
	m_sql_query->prepare(QString("DELETE FROM Menu WHERE Id=?;"));
	m_sql_query->bindValue(0, id);

	if (!m_sql_query->exec())
		return false;
	else
		return true;
}



bool MenuDatabase::remove_category(const QString& category)
{
	m_sql_query->prepare(QString("DELETE FROM Menu WHERE Category=?;"));
	m_sql_query->bindValue(0, category);

	if (!m_sql_query->exec())
		return false;
	else
		return true;
}



bool MenuDatabase::change_name(int id, const QString& name)
{
	m_sql_query->prepare(QString("UPDATE Menu SET Name=? WHERE Id=?;"));
	m_sql_query->bindValue(0, name);
	m_sql_query->bindValue(1, id);

	if (!m_sql_query->exec())
		return false;
	else
		return true;
}



bool MenuDatabase::change_img(int id, const QImage& img)
{
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);
	img.save(&buffer, "PNG");

	m_sql_query->prepare(QString("UPDATE Menu SET Icon=? WHERE Id=?;"));
	m_sql_query->bindValue(0, QVariant(ba));
	m_sql_query->bindValue(1, QVariant(id));

	if (!m_sql_query->exec())
		return false;
	else
		return true;
}



bool MenuDatabase::rename_category(const QString& before, const QString& after)
{
	m_sql_query->prepare("UPDATE Menu SET Category=? WHERE Category=?;");
	m_sql_query->bindValue(0, after);
	m_sql_query->bindValue(1, before);
	m_sql_query->exec();

	if (!m_sql_query->exec())
		return false;
	else
		return true;
}



bool MenuDatabase::query_items(QVector<MenuItem>& items) const
{
	items.clear();

	m_sql_query->exec("SELECT * FROM Menu ORDER BY rowid;");
	if (!m_sql_query->isActive())
		return false;

	while (m_sql_query->next()) {
		MenuItem item;
		item.id = m_sql_query->value(MenuItem::Id).toInt();
		item.name = m_sql_query->value(MenuItem::Name).toString();
		item.category = m_sql_query->value(MenuItem::Category).toString();
		item.img.loadFromData(m_sql_query->value(MenuItem::Image).toByteArray());
		items.append(item);
	}

	return true;
}