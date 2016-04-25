#ifndef _CLOSED_TABLE_WIDGET_H_
#define _CLOSED_TABLE_WIDGET_H_

#include <qframe.h>
#include <qstringlistmodel.h>
#include "ui_closed_table_widget.h"
#include "include/ordersrepo.h"


class ClosedTableWidget : public QFrame
{
	Q_OBJECT

public:
	ClosedTableWidget(OrdersRepo* repo, QWidget* parent = 0);
	~ClosedTableWidget();

public slots:
	void on_close_button_clicked();
	void save_table_info(const QList<Order*>& info);
	const QList<Order>& last_closed_table() const { return m_orders; }

private:
	Ui::closed_table_widget ui;
	QList<Order> m_orders;
	OrdersRepo* m_repo;
	QStringListModel m_string_list_model;

signals:
	void undo_btn_clicked();
};

#endif