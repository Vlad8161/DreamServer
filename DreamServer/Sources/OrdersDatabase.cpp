#include "..\include\OrdersDatabase.h"

#include <assert.h>
#include <qvector.h>
#include <qdatetime.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qdebug.h>


SqlMaker::SqlMaker(QSqlQuery* query, QString text) 
{
	m_query = query;
	m_text = text;
}



void SqlMaker::run()
{
	QString cur_time;
	cur_time = QTime::currentTime().toString("hh:mm:ss");
	qDebug() << "SqlMaker::run() : \"" << m_text << "\" begin at " << cur_time;
	m_query->exec(m_text);
	cur_time = QTime::currentTime().toString("hh:mm:ss");
	qDebug() << "SqlMaker::run() : \"" << m_text << "\" end at " << cur_time;
    if (!m_query->isActive())
        qDebug() << m_query->lastError();
	assert(m_query->isActive());
}



OrdersDatabase::OrdersDatabase(QSqlDatabase db)
{
	m_sql_query = new QSqlQuery(db);
	m_pool.setMaxThreadCount(1);

	m_sql_query->exec("SELECT * FROM Orders;");
	if (!m_sql_query->isActive()) {
		m_sql_query->exec("CREATE TABLE Orders("
			"Id INT UNIQUE, "
			"Name TEXT, "
			"Count INT, "
			"TableNum INT, "
			"TimeGot TEXT, "
			"TimePrepared TEXT, "
			"Status INT, "
			"Notes TEXT)");
	}
}



OrdersDatabase::~OrdersDatabase()
{
	m_pool.waitForDone();
	delete m_sql_query;
}



void OrdersDatabase::query_orders(QVector<Order>& orders) const
{
	orders.clear();
	m_sql_query->exec("SELECT * FROM Orders ORDER BY Status, TimeGot;");
	if (m_sql_query->isActive()) {
		while (m_sql_query->next()) {
			Order o;
			o.id = m_sql_query->value(Order::Id).toInt();
			o.name = m_sql_query->value(Order::Name).toString();
			o.count = m_sql_query->value(Order::Count).toInt();
			o.table_num = m_sql_query->value(Order::TableNum).toInt();
			o.time_got = m_sql_query->value(Order::TimeGot).toString();
			o.time_prepared = m_sql_query->value(Order::TimePrepared).toString();
			o.status = m_sql_query->value(Order::Stat).toInt();
			o.notes = m_sql_query->value(Order::Notes).toString();
			orders.push_back(o);
		}
	}
}



void OrdersDatabase::add_order(const Order& order) 
{ 
	SqlMaker* maker = new SqlMaker(m_sql_query, QString("INSERT INTO Orders "
		"(Id, Name, Count, TableNum, TimeGot, TimePrepared, Status, Notes) VALUES "
		"(%1, '%2', %3,    %4,       '%5',    '%6',         %7,     '%8');")
		.arg(order.id)
		.arg(order.name)
		.arg(order.count)
		.arg(order.table_num)
		.arg(order.time_got)
		.arg(order.time_prepared)
		.arg(order.status)
		.arg(order.notes));
	m_pool.start(maker);
}



void OrdersDatabase::add_orders(QVector<Order>::iterator begin, QVector<Order>::iterator end)
{
	if (begin == end)
		return;

	QString str_query = "INSERT INTO Orders "
		"(Id, Name, Count, TableNum, TimeGot, TimePrepared, Status, Notes) VALUES ";

	for (auto i = begin; i != end; i++) {
		str_query.append(QString("(%1, '%2', %3, %4, '%5', '%6', %7, '%8'),")
			.arg(i->id)
			.arg(i->name)
			.arg(i->count)
			.arg(i->table_num)
			.arg(i->time_got)
			.arg(i->time_prepared)
			.arg(i->status)
			.arg(i->notes)
        );
	}
	str_query[str_query.size() - 1] = ';';

	SqlMaker* maker = new SqlMaker(m_sql_query, str_query);
	m_pool.start(maker);
}



void OrdersDatabase::change_order_status(int id, int new_status)
{
	SqlMaker* maker = new SqlMaker(m_sql_query, 
		QString("UPDATE Orders SET Status=%2 WHERE Id=%1;")
		.arg(id)
		.arg(new_status));
	m_pool.start(maker);
}



void OrdersDatabase::set_time_stamp(int id, QString time_stamp)
{
	SqlMaker* maker = new SqlMaker(m_sql_query, 
		QString("UPDATE Orders SET TimePrepared='%2' WHERE Id=%1;")
		.arg(id)
		.arg(time_stamp));
	m_pool.start(maker);
}



void OrdersDatabase::remove_order(int id)
{
	SqlMaker* maker = new SqlMaker(m_sql_query, 
		QString("DELETE FROM Orders WHERE Id=%1;").arg(id));
	m_pool.start(maker);
}



void OrdersDatabase::remove_all_table_orders(int id)
{
	SqlMaker* maker = new SqlMaker(m_sql_query, 
		QString("DELETE FROM Orders WHERE TableNum=%1;").arg(id));
	m_pool.start(maker);
}