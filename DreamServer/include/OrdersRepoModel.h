#ifndef _ORDERS_REPO_MODEL_H_
#define _ORDERS_REPO_MODEL_H_

#include <qabstractitemmodel.h>
#include "OrdersRepo.h"


class OrdersRepoModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	OrdersRepoModel(OrdersRepo* repo, int t_num);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant headerData(int section, Qt::Orientation o, int role) const override;
	void set_not_prepared(int row);
	void set_prepared(int row);
	void set_served(int row);

public slots:
	void on_repo_changed(int t_num);
	void on_order_added(int t_num, int row);
	void on_double_click(const QModelIndex& index);

private:
	OrdersRepo* m_repo;
	int m_table_num;
	QImage m_img_prepared;
	QImage m_img_not_prepared;
	QImage m_img_served;
};

#endif