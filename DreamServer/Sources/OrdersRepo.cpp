#include "include\OrdersRepo.h"
#include "include\OrdersRepoModel.h"
#include "include\MenuDatabase.h"

#include <QSqlQuery.h>
#include <QDebug.h>
#include <QDateTime.h>
#include <assert.h>
#include <algorithm>


OrdersRepo::OrdersRepo(QSqlDatabase db, MenuDatabaseModel* menu_db) : m_db(db)
{
	assert(menu_db != nullptr);

	m_menu_db = menu_db;

	QVector<Order> orders;
	m_db.query_orders(orders);
	m_cache = new OrdersRepoCache(orders);

	m_max_id = std::max_element(orders.begin(), orders.end(),
		[](const Order& o1, const Order& o2) {
		return o1.id < o2.id;
	})->id;
}



OrdersRepo::~OrdersRepo()
{
	delete m_cache;
}



// ���������� ������ ������
void OrdersRepo::add_order(int course_id, int t_num, int count, QString notes)
{
	Order o;
	MenuItem course;

	// ��������� ���������� � ����� �� ��� id
	course = m_menu_db->item(course_id);

	// �������� ������
	o.id = ++m_max_id;
	o.name = course.name;
	o.count = count;
	o.notes = notes;
	o.table_num = t_num;
	o.status = Order::NOT_PREPARED;
	o.time_got = QTime::currentTime().toString("hh:mm");

	// ���������� ������ � ��
	m_db.add_order(o); 

	// ���������� � ���
	m_cache->add_order(o);

	// ��������, ��� �������� ����� �����
	emit order_added(t_num);
}



bool OrdersRepo::add_orders(const QVector<PreOrder>& pre_orders)
{
	QVector<Order> orders = QVector<Order>(50);

	if (pre_orders.isEmpty())
		return true;

	int t_num = pre_orders[0].t_num;
	if (!std::all_of(pre_orders.begin(), pre_orders.end(), [t_num](const PreOrder& o) {
		return t_num == o.t_num;
	}))
		return false;

	auto iter_start = pre_orders.begin();
	while (iter_start != pre_orders.end()) {
		auto iter_end = std::min(pre_orders.end(), iter_start + 50);
		auto iter_insert_begin = orders.begin();
		auto iter_insert_end = std::transform(iter_start, iter_end, iter_insert_begin,
			[this](const PreOrder& o) {
			Order order;
			MenuItem course = m_menu_db->item(o.id_course);

			// �������� ������
			order.id = ++m_max_id;
			order.name = course.name;
			order.count = o.count;
			order.notes = o.notes;
			order.table_num = o.t_num;
			order.status = Order::NOT_PREPARED;
			order.time_got = QTime::currentTime().toString("hh:mm");

			m_cache->add_order(order);

			return order;
		});
		m_db.add_orders(iter_insert_begin, iter_insert_end);
		iter_start = iter_end;
	}

	emit order_added(t_num);

	return true;
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
        m_cache->add_order(*new_order);
	}
	emit table_changed(t_num);
}



// ��������� ����
bool OrdersRepo::close_table(int table_num)
{
	// ������ ��
	m_db.remove_all_table_orders(table_num);

    // ������ ���
    m_cache->remove_orders(table_num);

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

    const Order* o = m_cache->order(t_num, row);
    if (o != nullptr) {
        QString time_stamp;

        // ���������� ��������� �����
        if (o->status == Order::NOT_PREPARED && status != Order::NOT_PREPARED) {
            time_stamp = QTime::currentTime().toString("hh:mm");
        }
        else if (o->status != Order::NOT_PREPARED && status == Order::NOT_PREPARED) {
            time_stamp = QString("");
        }

        // ��������� ���� � ���
        m_db.change_order_status(o->id, status);
        m_db.set_time_stamp(o->id, time_stamp);
        m_cache->update_order(t_num, row, status, time_stamp);

        // �������, ��� ���� ���������
        emit table_changed(o->table_num);

        return true;
    }
    else {
        return false;
    }
}



OrdersRepoModel* OrdersRepo::create_model(int t_num)
{
	OrdersRepoModel* model = new OrdersRepoModel(this, t_num);

	connect(this, SIGNAL(table_changed(int)),
		model, SLOT(on_repo_changed(int)));
	connect(this, SIGNAL(order_added(int)),
		model, SLOT(on_order_added(int)));

	return model;
}