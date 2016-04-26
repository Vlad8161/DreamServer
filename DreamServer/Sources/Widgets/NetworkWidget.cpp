#include "include/widgets/networkwidget.h"
#include "include/networkconnectionsmodel.h"
#include <qhostaddress.h>


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