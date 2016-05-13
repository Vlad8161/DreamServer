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
		return o1.id > o2.id;
	})->id;
}



OrdersRepo::~OrdersRepo()
{
	delete m_cache;
}



// добавление нового заказа
void OrdersRepo::add_order(int course_id, int t_num, int count, QString notes)
{
	Order o;
	MenuItem course;

	// получение информации о блюде по его id
	course = m_menu_db->item(course_id);

	// создание заказа
	o.id = ++m_max_id;
	o.name = course.name;
	o.count = count;
	o.notes = notes;
	o.table_num = t_num;
	o.status = Order::NOT_PREPARED;
	o.time_got = QTime::currentTime().toString("hh:mm");

	// добавление заказа в БД
	m_db.add_order(o); 

	// добавление в кэш
	m_cache->add_order(o);

	// сигналим, что добавлен новый заказ
	emit order_added(t_num);
}



bool OrdersRepo::add_orders(const QVector<PreOrder>& pre_orders)
{
	QVector<Order*> orders = QVector<Order*>(50, nullptr);

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

			// создание заказа
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



// восстановление заказов
void OrdersRepo::restore_orders(const QList<Order>& orders)
{
	int t_num = orders.back().table_num;
	for (auto i = orders.begin(); i != orders.end(); i++) {
		assert(i->table_num == t_num);

		Order* new_order = new Order(*i);

		// Добавляем в БД
		m_db.add_order(*new_order);

		// Добавляем в список столов
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


// проверяем обслужен ли стол
bool OrdersRepo::is_table_served(int t_num)
{
	// убеждаемся, что все заказы обслужены, иначе выходим
	for (auto& i : m_table_orders[t_num]) {
		if (i->status != Order::SERVED)
			return false;
	}

	return true;
}



// закрываем стол
bool OrdersRepo::close_table(int table_num)
{
	// если стол не открыт, то выходим из метода
	if (!m_table_orders.contains(table_num) ||
		m_table_orders[table_num].isEmpty())
		return false;

	// если все таки все гуд, то поочередно удаляем заказы из репозитория, и чистим стол
	for (auto elem : m_table_orders[table_num]) {
		delete elem;
	}
	m_table_orders[table_num].clear();

	// чистим БД
	m_db.remove_all_table_orders(table_num);

	// говорим, что содержимое стола изменилось
	emit table_changed(table_num);

	return true;
}



// изменение статуса заказа
bool OrdersRepo::set_order_status(int t_num, int row, int status)
{
	// если статус неверный, то выходим
	if (status < 0 || status > 2)
		return false;

	// ищем нужный заказ и меняем его статус
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

	// меняем позицию заказа в столе так чтобы стол был отсортирован по статусу
	auto to = table.begin();
	auto end = table.end();
	while (to != end && (*to)->status <= o->status)
		to++;
	int i_from = row;
	int i_to = to - table.begin();
	if (i_to > i_from)
		i_to--;
	table.move(i_from, i_to);

	// говорим, что стол изменился
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
	connect(this, SIGNAL(order_added(int)),
		model, SLOT(on_order_added(int)));

	return model;
}