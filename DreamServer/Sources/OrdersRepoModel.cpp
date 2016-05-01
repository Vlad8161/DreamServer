#include "..\include\OrdersRepoModel.h"

#include <qbrush.h>


OrdersRepoModel::OrdersRepoModel(OrdersRepo* repo, int t_num)
	: m_repo(repo), m_table_num(t_num),
	m_img_not_prepared(":/Resources/Icons/close_7702.png"),
	m_img_prepared(":/Resources/Icons/apply.png"),
	m_img_served(":/Resources/Icons/add_8214.png")
{

}



int OrdersRepoModel::rowCount(const QModelIndex& index) const
{
	if (index.isValid()) {
		return 0;
	}
	else {
		return m_repo->n_table_orders(m_table_num);
	}
}



int OrdersRepoModel::columnCount(const QModelIndex& index) const
{
	if (index.isValid()) {
		return 0;
	}
	else {
		return 5;
	}
}



QVariant OrdersRepoModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	auto orders = m_repo->query_table_orders(m_table_num);

	int row = index.row();
	int col = index.column();

	if (row >= orders.size() ||
		col >= columnCount())
		return QVariant();
	
	if (role == Qt::DisplayRole) {
		switch (col)
		{

		case 1: {
			QString s;
			s = QString("%1 x%3")
				.arg(orders[row]->name)
				.arg(orders[row]->count);
			return QVariant(s);
			break;
		}

		case 2: {
			return QVariant(orders[row]->time_got);
			break;
		}

		case 3: {
			return QVariant(orders[row]->time_prepared);
			break;
		}

		case 4: {
			return QVariant(orders[row]->notes);
			break;
		}

		default:
			return QVariant();
			break;
		}

	}

	if (role == Qt::DecorationRole && col == 0) {
		int status = orders[row]->status;
		if (status == Order::NOT_PREPARED) {
			return QVariant();
		}
		else if (status == Order::PREPARED) {
			return QVariant(m_img_prepared);
		}
		else {
			return QVariant(m_img_served);
		}
	}

	// этот кусок нужен дл€ раскраски четных и нечетных строк в разные цвета
	if (role == Qt::BackgroundColorRole) {
		if (row & 1)
			return QBrush(QColor(236, 248, 255));
		else
			return QBrush(QColor(220, 234, 255));
	}

	if (role == Qt::TextAlignmentRole) {
		if (col == 2 || col == 3) {
			return QVariant(Qt::AlignCenter);
		}
	}
	
	return QVariant();
}



void OrdersRepoModel::on_repo_changed(int t_num)
{
	if (t_num != m_table_num)
		return;

	beginResetModel();
	endResetModel();
}



void OrdersRepoModel::on_order_added(int t_num)
{
	if (t_num != m_table_num)
		return;

	beginResetModel();
	endResetModel();
}



// обработка довйного клика по записи
void OrdersRepoModel::on_double_click(const QModelIndex& index)
{
	const Order* o = m_repo->order(m_table_num, index.row());
	int prev_status = o->status;

	// если заказ не готов, то делаем его готовым
	if (prev_status == Order::NOT_PREPARED) {
		m_repo->set_order_status(m_table_num, index.row(), Order::PREPARED);
	}
	// если заказ готов, то делаем его обслуженным
	else if (prev_status == Order::PREPARED) {
		m_repo->set_order_status(m_table_num, index.row(), Order::SERVED);
	}
}



QVariant OrdersRepoModel::headerData(int section, Qt::Orientation o, int role) const
{
	if (role == Qt::DisplayRole) {
		if (section == 0) {
			return QVariant();
		}

		if (section == 1) {
			return QVariant(QString::fromLocal8Bit("Ќазвание"));
		}

		if (section == 2) {
			return QVariant(QString::fromLocal8Bit("ѕолучено в"));
		}

		if (section == 3) {
			return QVariant(QString::fromLocal8Bit("√отово в"));
		}

		if (section == 4) {
			return QVariant(QString::fromLocal8Bit("«аметки"));
		}
	}

	return QVariant();
}



Qt::ItemFlags OrdersRepoModel::flags(const QModelIndex& index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}



// три следующие слота обрабатывают запросы на установку конкретного статуса заказа
void OrdersRepoModel::set_served(int row)
{
	m_repo->set_order_status(m_table_num, row, Order::SERVED);
}



void OrdersRepoModel::set_prepared(int row)
{
	m_repo->set_order_status(m_table_num, row, Order::PREPARED);
}



void OrdersRepoModel::set_not_prepared(int row)
{
	m_repo->set_order_status(m_table_num, row, Order::NOT_PREPARED);
}