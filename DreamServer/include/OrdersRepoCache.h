#ifndef _ORDERS_REPO_CACHE_H_
#define _ORDERS_REPO_CACHE_H_


#include <qobject.h>
#include <qmap.h>
#include <include/Order.h>


class OrdersRepoCache : public QObject
{
	Q_OBJECT

public:
	OrdersRepoCache(const QVector<Order>& orders);
	~OrdersRepoCache();
	QList<Order*> query_table_orders(int t_num) const;
	QList<int> opened_tables() const { return m_table_orders.keys(); }
	const Order* order(int t_num, int row) const;
	int n_table_orders(int t_num) const;
	void add_order(const Order& order);
	void add_orders(const QList<Order>& orders);
	void remove_orders(int table_num);
	void update_order(int t_num, int row, int status, const QString& time_prepared);
	bool is_table_served(int t_num);

private:
	QMap < int, QList<Order*> > m_table_orders;
};


#endif