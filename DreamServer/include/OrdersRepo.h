#ifndef _ORDERS_REPO_H_
#define _ORDERS_REPO_H_


#include "OrdersDatabase.h"
#include "OrdersRepoCache.h"
#include "MenuDatabaseModel.h"

class OrdersRepoModel;

// OrdersRepo - ����� �������������� ��������� ������� �������
// ������������� ��������� � ������� �� id, ���������� 
// ������ ������� � ���������� ������, ������������� ����������� 
// ��������� ������, ��������� �����, �������� ������ �������
class OrdersRepo : public QObject
{
	Q_OBJECT

public:
	OrdersRepo(QSqlDatabase db, MenuDatabaseModel* menu_db);
	~OrdersRepo();
    /**
     * ����� ����������� ������ ����� OrdersRepo �� 
     * �������� ������� ������������� ��������, ������� ��
     * ������ ������� � ��� OrderRepo � ����� ����
     * ������� � ���� ��� ��������� ������ �����������
     */
	QList<Order*> query_table_orders(int t_num) const { return m_cache->query_table_orders(t_num); }
	QList<int> opened_tables() const { return m_cache->opened_tables(); }
	const Order* order(int t_num, int row) const { return m_cache->order(t_num, row); }
	int n_table_orders(int t_num) const { return m_cache->n_table_orders(t_num); }
	void add_order(int course_id, int t_num, int count, QString notes);
	bool add_orders(const QVector<PreOrder>& orders);
	bool set_order_status(int t_num, int row, int status);
	bool close_table(int table_num);
    bool is_table_served(int t_num) const { return m_cache->is_table_served(t_num); }
	void restore_orders(const QList<Order>& orders);

	//��������� ������� ���������� ������ ��� ������ �����
	OrdersRepoModel* create_model(int t_num);

private:
	OrdersDatabase m_db;
	MenuDatabaseModel *m_menu_db;
	OrdersRepoCache* m_cache;
	int m_max_id;


signals:
	void order_added(int t_num);
	void table_changed(int);
};


#endif