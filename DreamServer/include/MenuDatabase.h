#ifndef _MENUDATABASE_H
#define _MENUDATABASE_H

#include "include/menuitem.h"
#include <qsqldatabase.h>


// этот класс предоставляет удобный интерфейс для работы с базой данных меню
class MenuDatabase
{
public:
	MenuDatabase(QSqlDatabase db);
	~MenuDatabase();
	bool add_menu_item(const MenuItem& item);
	bool remove_menu_item(int id);
	bool remove_category(const QString& category);
	bool change_name(int id, const QString& name);
	bool change_img(int id, const QImage& img);
	bool rename_category(const QString& before, const QString& after);
	bool query_items(QVector<MenuItem>& items) const;

private:
	QSqlQuery* m_sql_query;
};

#endif