#include "include/OrdersRepoCache.h"
#include <qdatetime.h>
#include <qvector.h>
#include <algorithm>
#include <assert.h>

OrdersRepoCache::OrdersRepoCache(const QVector<Order>& orders)
{
	for (auto i = orders.begin(); i != orders.end(); i++) {
		Order* new_order = new Order(*i);

		if (!m_table_orders.contains(i->table_num)) {
			m_table_orders[i->table_num] = QList<Order*>();
		}

		m_table_orders[i->table_num].push_back(new_order);
	}
}



OrdersRepoCache::~OrdersRepoCache()
{
	for (auto& i : m_table_orders) {
		for (auto j : i)
			delete j;
	}
}



QList<Order*> OrdersRepoCache::query_table_orders(int t_num) const
{
	if (!m_table_orders.contains(t_num)) {
		return QList<Order*>() ;
	}
	else {
		return m_table_orders[t_num];
	}
}



const Order* OrdersRepoCache::order(int t_num, int row) const
{
	if (!m_table_orders.contains(t_num) || row >= m_table_orders[t_num].size()) {
		return nullptr;
	}
	else {
		return m_table_orders[t_num][row];
	}
}



int OrdersRepoCache::n_table_orders(int t_num) const
{
	if (!m_table_orders.contains(t_num)) {
		return 0;
	}
	else {
		return m_table_orders[t_num].size();
	}
}



void OrdersRepoCache::add_order(const Order& order)
{
	Order* new_order = new Order(order);
	// начинаем добавлять новый заказ в список столов
	// если стола с таким номером еще нет в списке, то добавляем новый
	if (!m_table_orders.contains(new_order->table_num))
		m_table_orders[new_order->table_num] = QList<Order*>();

	// здесь вставляем элемент в нужную позицию
	QList<Order*>& table_to_insert = m_table_orders[new_order->table_num];
	table_to_insert.insert(
		std::find_if(table_to_insert.begin(), table_to_insert.end(),
			[new_order](const Order* o) { return new_order->status > o->status; }),
		new_order);
}



// восстановление заказов
void OrdersRepoCache::add_orders(const QList<Order>& orders)
{
	if (orders.isEmpty())
		return;

	for (auto i = orders.begin(); i != orders.end(); i++) {
		Order* new_order = new Order(*i);

		// Если стола еще нет, создаем его
		if (!m_table_orders.contains(new_order->table_num))
			m_table_orders[new_order->table_num] = QList < Order* > () ;

		// Добавляем в список столов
		QList<Order*>& table_to_insert = m_table_orders[new_order->table_num];
		table_to_insert.insert(
			std::find_if(table_to_insert.begin(), table_to_insert.end(),
				[new_order](const Order* o) { return new_order->status > o->status; }),
			new_order);
	}
}



// удаление заказов с определенного стола
void OrdersRepoCache::remove_orders(int table_num)
{
	// Если стол пустой выходим
	if (!m_table_orders.contains(table_num) ||
		m_table_orders[table_num].isEmpty())
		return;

	// Иначе удаляем все нафиг
	for (auto& i : m_table_orders[table_num]) {
		delete i;
	}
	m_table_orders[table_num].clear();
}



// проверка обслужен ли стол
bool OrdersRepoCache::is_table_served(int t_num)
{
	return std::all_of(m_table_orders[t_num].begin(), m_table_orders[t_num].end(),
		[t_num](const Order* o) { return o->status == Order::SERVED; });
}



// меняем статус указанного заказа
void OrdersRepoCache::update_order(int t_num, int row, int status, const QString& time_prepared)
{
	assert(m_table_orders.contains(t_num));
	assert(row < m_table_orders[t_num].size());
	assert(!m_table_orders[t_num].isEmpty());

	// обновляем информацию о заказе
	Order* order_to_update = m_table_orders[t_num][row];
	order_to_update->time_prepared = time_prepared;
	order_to_update->status = status;

	// сортируем по статусу
	QList<Order*>& table = m_table_orders[t_num];
	auto to = table.begin();
	auto end = table.end();
	while (to != end && (*to)->status <= status)
		to++;
	int i_from = row;
	int i_to = to - table.begin();
	if (i_to > i_from)
		i_to--;
	table.move(i_from, i_to);
}