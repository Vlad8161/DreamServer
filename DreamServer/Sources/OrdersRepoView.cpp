#include "include\OrdersRepoView.h"
#include "include/OrdersRepoModel.h"
#include <qheaderview.h>
#include <qdebug.h>
#include <qmenu.h>
#include <assert.h>


OrdersRepoView::OrdersRepoView(OrdersRepoModel* model)
{
	assert(model != nullptr);
	m_model = model;
	this->setModel(m_model);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	this->verticalHeader()->hide();
	this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	this->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
	this->horizontalHeader()->resizeSection(2, 40);
	this->horizontalHeader()->resizeSection(0, 30);

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)),
		m_model, SLOT(on_double_click(const QModelIndex&)));

	m_action_not_prepare = new QAction(QString::fromLocal8Bit("Не готово"), this);
	connect(m_action_not_prepare, SIGNAL(triggered(bool)),
		this, SLOT(on_action_not_prepare()));

	m_action_prepare = new QAction(QString::fromLocal8Bit("Готово"), this);
	connect(m_action_prepare, SIGNAL(triggered(bool)),
		this, SLOT(on_action_prepare()));

	m_action_serve = new QAction(QString::fromLocal8Bit("Обслужено"), this);
	connect(m_action_serve, SIGNAL(triggered(bool)),
		this, SLOT(on_action_serve()));

	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(on_context_menu_requested(const QPoint&)));

	m_context_menu = new QMenu(this);
	m_context_menu->addAction(m_action_not_prepare);
	m_context_menu->addAction(m_action_prepare);
	m_context_menu->addAction(m_action_serve);
}



OrdersRepoView::~OrdersRepoView()
{
	delete m_model;
	delete m_context_menu;
}



void OrdersRepoView::on_action_not_prepare()
{
	int row = rowAt(m_cursor_pos.y());
	if (row == -1)
		return;
	m_model->set_not_prepared(row);
}



void OrdersRepoView::on_action_prepare()
{
	int row = rowAt(m_cursor_pos.y());
	if (row == -1)
		return;
	m_model->set_prepared(row);
}



void OrdersRepoView::on_action_serve()
{
	int row = rowAt(m_cursor_pos.y());
	if (row == -1)
		return;
	m_model->set_served(row);
}



void OrdersRepoView::on_context_menu_requested(const QPoint& pos)
{
	m_cursor_pos = pos;
	m_context_menu->exec(mapToGlobal(pos));
}