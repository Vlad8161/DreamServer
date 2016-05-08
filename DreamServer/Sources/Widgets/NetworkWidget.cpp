#include "include/widgets/networkwidget.h"
#include "include/networkconnectionsmodel.h"
#include <qhostaddress.h>
#include <qmenu.h>


NetworkWidget::NetworkWidget(NetworkManager* network_manager, QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_mgr = network_manager;
	m_model = network_manager->create_model();

	ui.connections_view->setModel(m_model);

	QHostAddress address = m_mgr->address();
	if (address == QHostAddress::Null) {
		ui.label_adderss->setText("Server is not running");
	}
	else {
		ui.label_adderss->setText(QString("%1:%2").arg(address.toString()).arg(m_mgr->port()));
	}

	m_action_server_start = new QAction(this);
	m_action_server_start->setText(QString::fromLocal8Bit("Запуск сервера"));
	m_action_server_start->setIcon(QIcon(":/Resources/Icons/start.png"));
	m_action_server_start->setCheckable(true);
	m_action_server_start->setChecked(false);
	connect(m_action_server_start, SIGNAL(triggered(bool)),
		this, SLOT(on_action_server_start()));

	m_action_server_stop = new QAction(this);
	m_action_server_stop->setText(QString::fromLocal8Bit("Остановка сервера"));
	m_action_server_stop->setIcon(QIcon(":/Resources/Icons/stop.png"));
	m_action_server_stop->setCheckable(true);
	m_action_server_stop->setChecked(true);
	connect(m_action_server_stop, SIGNAL(triggered(bool)),
		this, SLOT(on_action_server_stop()));

	m_action_kick = new QAction(QString::fromLocal8Bit("Выгнать нахрен"), this);
	connect(m_action_kick, SIGNAL(triggered(bool)),
		this, SLOT(on_action_kick()));

	ui.connections_view->setContextMenuPolicy(Qt::CustomContextMenu);
	m_context_menu = new QMenu(this);
	m_context_menu->addAction(m_action_kick);
	connect(ui.connections_view, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(on_context_menu_requested(const QPoint&)));
}



NetworkWidget::~NetworkWidget()
{
	delete m_model;
	delete m_mgr;
}



void NetworkWidget::on_action_server_start()
{
	if (m_mgr->start()) {
		m_action_server_start->setChecked(true);
		m_action_server_stop->setChecked(false);
		QHostAddress address = m_mgr->address();
		ui.label_adderss->setText(QString("%1:%2")
			.arg(address.toString())
			.arg(m_mgr->port()));
	}
	else {
		m_action_server_start->setChecked(false);
		m_action_server_stop->setChecked(true);
	}
}



void NetworkWidget::on_action_server_stop()
{
	m_mgr->stop();
	m_action_server_start->setChecked(false);
	m_action_server_stop->setChecked(true);
	ui.label_adderss->setText("Server is not running");
}



void NetworkWidget::on_action_kick()
{
	auto selected_rows = ui.connections_view->selectionModel()->selectedRows();
	if (selected_rows.isEmpty())
		return;

	m_mgr->kick_client_at_row(selected_rows[0].row());
}


 
void NetworkWidget::on_context_menu_requested(const QPoint& pos)
{
	m_cursor_pos = pos;
	m_context_menu->exec(mapToGlobal(pos));
}