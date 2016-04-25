#include "include\widgets\TableContainerWidget.h"
#include <cassert>
#include <QResizeEvent>

TableContainerWidget::TableContainerWidget(QWidget *parent)
	: QFrame(parent)
{

}



TableContainerWidget::~TableContainerWidget()
{

}



void TableContainerWidget::append_table(TableWidget* new_table)
{
	assert(new_table != nullptr);

	bool is_exists = false;
	for (auto i = m_tables.begin(); i != m_tables.end(); i++){
		if (*i == new_table) {
			is_exists = true;
			break;
		}
	}

	if (!is_exists) {
		new_table->setParent(this);
		m_tables.append(new_table);
	}
}



void TableContainerWidget::remove_table()
{
	if (m_tables.size() != 0)
		m_tables.removeLast();
}



void TableContainerWidget::remove_table(int i)
{
	if (m_tables.size() != 0 && i >= 0 && i < m_tables.size())
		m_tables.removeAt(i);
}



void TableContainerWidget::resizeEvent(QResizeEvent* e)
{
	int n; // количество виджетов в строке
	int wi; // ширина одного виджета
	int hi; // высота одного виджета

	QSize new_size = e->size();
	QSize min_size = m_tables[0]->minimumSize();

	n = (new_size.width() - m_margin_left - m_margin_right + m_spacing) 
		/ (min_size.width() + m_spacing); 
	wi = (new_size.width() - m_margin_left - m_margin_right - (n - 1) * m_spacing) / n; 
	hi = min_size.height(); 

	for (int i = 0; i < m_tables.size(); i++) {
		QRect new_r;
		int col = i % n;
		int row = i / n;
		new_r.setLeft(m_margin_left + col * (wi + m_spacing));
		new_r.setTop(m_margin_top + row * (hi + m_spacing));
		new_r.setRight(new_r.left() + wi);
		new_r.setBottom(new_r.top() + hi);
		m_tables[i]->setGeometry(new_r);
	}
}