#ifndef _ORDERS_DATABASE_H_
#define _ORDERS_DATABASE_H_

#include <qthreadpool.h>
#include <qsqldatabase.h>
#include "Order.h"

class QSqlQuery;

// Класс, необходимый для выполнения SQL-запросов к БД в отдельном потоке
class SqlMaker : public QRunnable
{
public:
	SqlMaker(QSqlQuery* query, QString text);
	void run() override;

private:
	QSqlQuery* m_query;
	QString m_text;
};



// простой адаптер для работы с БД
class OrdersDatabase
{
public:
	OrdersDatabase(QSqlDatabase db);
	~OrdersDatabase();
	void add_order(const Order& order);
	void change_order_status(int id, int new_status);
	void set_time_stamp(int id, QString time_stamp);
	void remove_order(int id);
	void remove_all_table_orders(int t_num);
	void query_orders(QVector<Order>& orders) const;

private:
	QThreadPool m_pool;
	QSqlQuery* m_sql_query;
};

#endif