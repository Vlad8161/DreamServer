#include "include\OrdersRepo.h"
#include "include\OrdersRepoModel.h"
#include "include\MenuDatabase.h"

#include <QSqlQuery.h>
#include <QDebug.h>
#include <QDateTime.h>
#include <assert.h>


OrdersRepo::OrdersRepo(QSqlDatabase db, MenuDatabaseModel* menu_db) : m_db(db)
{
	assert(menu_db != nullptr);
	m_menu_db = menu_db;
	m_init_orders_list();
}



OrdersRepo::~OrdersRepo()
{
	for (auto& i : m_table_orders) {
		for (auto j : i)
			delete j;
	}
}


// ����� ��������������� �� ���� ������ ������, ������������ ���
// � �������������� ���������� m_max_id, ����������� ���
// ��������� ������ ������
void OrdersRepo::m_init_orders_list()
{
	QVector<Order> orders;

	m_max_id = 0;

	m_db.query_orders(orders);

	for (auto i = orders.begin(); i != orders.end(); i++) {
		Order* new_order = new Order(*i);

		if (new_order->id > m_max_id)
			m_max_id = new_order->id;

		if (!m_table_orders.contains(i->table_num)) {
			m_table_orders[i->table_num] = QList<Order*>();
		}

		m_table_orders[i->table_num].push_back(new_order);
	}
}



QList<Order*> OrdersRepo::query_table_orders(int t_num) const
{
	if (!m_table_orders.contains(t_num)) {
		return QList<Order*>() ;
	}
	else {
		return m_table_orders[t_num];
	}
}



const Order* OrdersRepo::order(int t_num, int row) const
{
	assert(m_table_orders.contains(t_num));
	assert(row < m_table_orders[t_num].size());
	return m_table_orders[t_num][row];
}



// ���������� ������ ������
void OrdersRepo::add_order(int course_id, int t_num, int count, QString notes)
{
	Order* new_order = new Order;
	MenuItem course;

	// ��������� ���������� � ����� �� ��� id
	course = m_menu_db->item(course_id);

	// �������� ������
	new_order->id = ++m_max_id;
	new_order->name = course.name;
	new_order->count = count;
	new_order->notes = notes;
	new_order->table_num = t_num;
	new_order->status = Order::NOT_PREPARED;
	new_order->time_got = QTime::currentTime().toString("hh:mm");

	// ���������� ������ � ��
	m_db.add_order(*new_order); 

	// �������� ��������� ����� ����� � ������ ������
	// ���� ����� � ����� ������� ��� ��� � ������, �� ��������� �����
	if (!m_table_orders.contains(new_order->table_num)) {
		m_table_orders[new_order->table_num] = QList<Order*>();
	}

	// ����� ���� ����������� ������� ��� ������ ������ � ��������� ��� ����
	int row_num = 0;
	auto i = m_table_orders[new_order->table_num].begin();
	auto end = m_table_orders[new_order->table_num].end();
	while (i != end && (*i)->status <= new_order->status) {
		row_num++;
		i++;
	}
	m_table_orders[new_order->table_num].insert(i, new_order);

	// ��������, ��� �������� ����� �����
	emit order_added(new_order->table_num, row_num);
}



// �������������� �������
void OrdersRepo::restore_orders(const QList<Order>& orders)
{
	int t_num = orders.back().table_num;
	for (auto i = orders.begin(); i != orders.end(); i++) {
		assert(i->table_num == t_num);

		Order* new_order = new Order(*i);

		// ��������� � ��
		m_db.add_order(*new_order);

		// ��������� � ������ ������
		int row_num = 0;
		auto j = m_table_orders[new_order->table_num].begin();
		auto end = m_table_orders[new_order->table_num].end();
		while (j != end && (*j)->status <= new_order->status) {
			row_num++;
			j++;
		}
		m_table_orders[new_order->table_num].insert(j, new_order);
	}
	emit table_changed(t_num);
}



// ��������� ����
bool OrdersRepo::close_table(int table_num)
{
	// ���� ���� �� ������, �� ������� �� ������
	if (!m_table_orders.contains(table_num) ||
		m_table_orders[table_num].isEmpty())
		return false;

	// ����������, ��� ��� ������ ���������, ����� �������
	for (auto& i : m_table_orders[table_num]) {
		if (i->status != Order::SERVED)
			return false;
	}

	// ���� ��� ���� ��� ���, �� ���������� ������� ������ �� �����������, � ������ ����
	for (auto elem : m_table_orders[table_num]) {
		delete elem;
	}
	m_table_orders[table_num].clear();

	// ������ ��
	m_db.remove_all_table_orders(table_num);

	// �������, ��� ���������� ����� ����������
	emit table_changed(table_num);

	return true;
}



// ��������� ������� ������
bool OrdersRepo::set_order_status(int t_num, int row, int status)
{
	// ���� ������ ��������, �� �������
	if (status < 0 || status > 2)
		return false;

	// ���� ������ ����� � ������ ��� ������
	assert(m_table_orders.contains(t_num));
	auto& table = m_table_orders[t_num];
	assert(row < table.size());

	Order* o = table[row];
	int old_status = o->status;
	o->status = status;
	m_db.change_order_status(o->id, status);
	if (old_status == Order::NOT_PREPARED && status != Order::NOT_PREPARED) {
		o->time_prepared = QTime::currentTime().toString("hh:mm");
		m_db.set_time_stamp(o->id, o->time_prepared);
	}
	else if (old_status != Order::NOT_PREPARED && status == Order::NOT_PREPARED) {
		o->time_prepared = QString("");
		m_db.set_time_stamp(o->id, o->time_prepared);
	}

	// ������ ������� ������ � ����� ��� ����� ���� ��� ������������ �� �������
	auto to = table.begin();
	auto end = table.end();
	while (to != end && (*to)->status <= o->status)
		to++;
	int i_from = row;
	int i_to = to - table.begin();
	if (i_to > i_from)
		i_to--;
	table.move(i_from, i_to);

	// �������, ��� ���� ���������
	emit table_changed(o->table_num);

	return true;
}



int OrdersRepo::n_table_orders(int t_num) const
{
	if (m_table_orders.contains(t_num)) {
		return m_table_orders[t_num].size();
	}
	else {
		return 0;
	}
}



OrdersRepoModel* OrdersRepo::create_model(int t_num)
{
	OrdersRepoModel* model = new OrdersRepoModel(this, t_num);

	connect(this, SIGNAL(table_changed(int)),
		model, SLOT(on_repo_changed(int)));
	connect(this, SIGNAL(order_added(int, int)),
		model, SLOT(on_order_added(int, int)));

	return model;
}