#ifndef _ORDERS_REPO_H_
#define _ORDERS_REPO_H_


#include "OrdersDatabase.h"
#include "MenuDatabaseModel.h"

class OrdersRepoModel;

// OrdersRepo - класс представляющий хранилище текущих заказов
// предоставляет интерфейс к заказам по id, возвращает 
// списки заказов к конкретным столам, предоставляет возможность 
// добавлять заказы, закрывать столы, изменять статус заказов
class OrdersRepo : public QObject
{
	Q_OBJECT

public:
	OrdersRepo(QSqlDatabase db, MenuDatabaseModel* menu_db);
	~OrdersRepo();
	QList<Order*> query_table_orders(int t_nums) const;
	QList<int> opened_tables() const { return m_table_orders.keys(); }
	const Order* order(int t_num, int row) const;
	int n_table_orders(int t_num) const;
	void add_order(int course_id, int t_num, int count, QString notes);
	bool set_order_status(int t_num, int row, int status);
	bool close_table(int table_num);
	void restore_orders(const QList<Order>& orders);

	//Следующая функция возвращает модель для одного стола
	OrdersRepoModel* create_model(int t_num);

private:
	void m_init_orders_list();

private:
	OrdersDatabase m_db;
	MenuDatabaseModel *m_menu_db;
	QMap<int, QList<Order*> > m_table_orders;
	int m_max_id;

signals:
	void order_added(int t_num, int row);
	void table_changed(int);
};


#endif