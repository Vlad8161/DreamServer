#include "include/widgets/closedtablewidget.h"
#include <assert.h>


ClosedTableWidget::ClosedTableWidget(OrdersRepo* repo, QWidget* parent) 
	: QFrame(parent), m_repo(repo)
{
	ui.setupUi(this);
	ui.list_view->setModel(&m_string_list_model);
	ui.list_view->setSelectionMode(QAbstractItemView::NoSelection);
	ui.list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(ui.undo_button, SIGNAL(clicked()),
		this, SIGNAL(undo_btn_clicked()));
}


ClosedTableWidget::~ClosedTableWidget()
{
	
}



void ClosedTableWidget::on_close_button_clicked()
{
	m_string_list_model.removeRows(0, m_string_list_model.rowCount());
	m_orders.clear();
	this->hide();
}



void ClosedTableWidget::save_table_info(const QList<Order*>& info)
{
	m_orders.clear();
	QStringList lst;

	for (auto& i : info) {
		m_orders.push_back(*i);
		lst.push_back(QString("%1 x%3")
			.arg(i->name)
			.arg(i->count));
	}
	
	m_string_list_model.setStringList(lst);
}