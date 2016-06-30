#include "include/widgets/networkwidget.h"
#include "include/networkconnectionsmodel.h"
#include <qhostaddress.h>
#include <qmenu.h>
#include <qsettings.h>
#include <qnetworkinterface.h>


NetworkWidget::NetworkWidget(NetworkManager* network_manager, QWidget* parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_mgr = network_manager;
	m_model = network_manager->create_model();

	ui.connections_view->setModel(m_model);

    auto interfaces = QNetworkInterface::allInterfaces();

    QVector < QPair < QString, QString > > v;
    for (auto & i : interfaces) {
        auto addresses = i.addressEntries();
        auto real_addr = std::find_if(addresses.begin(), addresses.end(),
            [](QNetworkAddressEntry & a) {
            return a.ip().toIPv4Address() != 0;
        });

        if (real_addr != addresses.end()) {
            v.push_back(QPair < QString, QString >(
                i.humanReadableName(),
                real_addr->ip().toString()
            ));
        }
    }
    v.push_front(QPair < QString, QString >(
        "localhost",
        "127.0.0.1"
    ));

    for (auto & i : v) {
        ui.select_interface->addItem(i.first, i.second);
    }

    QSettings settings("dpiki", "dreamserver");
    QString last_interfase_name = settings
        .value("NetworkWidget/last_interface", "localhost")
        .toString();
    auto found = std::find_if(v.begin(), v.end(),
        [&last_interfase_name](QPair < QString, QString > i) {
        return i.first == last_interfase_name;
    });

    if (found != v.end())
        ui.select_interface->setCurrentIndex(found - v.begin());

    m_mgr->set_address(QHostAddress(ui.select_interface->currentData().toString()));

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

	m_action_kick = new QAction(QString::fromLocal8Bit("Выгнать"), this);
	m_action_call = new QAction(QString::fromLocal8Bit("Вызвать"), this);

	ui.connections_view->setContextMenuPolicy(Qt::CustomContextMenu);
	m_context_menu = new QMenu(this);
	m_context_menu->addAction(m_action_call);
	//m_context_menu->addAction(m_action_kick);

    connect(
        ui.select_interface,
        static_cast <void(QComboBox::*)(const QString&)> (&QComboBox::currentIndexChanged),
        this,
        &NetworkWidget::on_combo_box_index_changed);
	connect(m_action_server_stop, SIGNAL(triggered(bool)),
		this, SLOT(on_action_server_stop()));
	connect(m_action_kick, SIGNAL(triggered(bool)),
		this, SLOT(on_action_kick()));
	connect(m_action_call, SIGNAL(triggered(bool)),
		this, SLOT(on_action_call()));
	connect(ui.connections_view, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(on_context_menu_requested(const QPoint&)));
    connect(ui.connections_view, &QAbstractItemView::doubleClicked,
        this, &NetworkWidget::on_list_view_double_click);
    connect(m_mgr, &NetworkManager::server_started,
        this, &NetworkWidget::on_server_started);
    connect(m_mgr, &NetworkManager::server_stopped,
        this, &NetworkWidget::on_server_stopped);
}



NetworkWidget::~NetworkWidget()
{
    QSettings settings("dpiki", "dreamserver");
    settings.setValue("NetworkWidget/last_interface", ui.select_interface->currentText());

	delete m_model;
	delete m_mgr;
}



void NetworkWidget::on_action_server_start()
{
    m_mgr->start();
}



void NetworkWidget::on_action_server_stop()
{
	m_mgr->stop();
}



void NetworkWidget::on_server_started()
{
    m_action_server_start->setChecked(true);
    m_action_server_stop->setChecked(false);
    QHostAddress address = m_mgr->address();
    ui.label_adderss->setText(QString("%1:%2")
        .arg(address.toString())
        .arg(m_mgr->port()));
}



void NetworkWidget::on_server_stopped()
{
	m_action_server_start->setChecked(false);
	m_action_server_stop->setChecked(true);
	ui.label_adderss->setText("Server is not running");
}



void NetworkWidget::on_combo_box_index_changed(const QString&)
{
    QString current_ip = ui.select_interface->currentData().toString();
    QHostAddress addr(current_ip);
    m_mgr->set_address(addr);
}



void NetworkWidget::on_action_kick()
{
	auto selected_rows = ui.connections_view->selectionModel()->selectedRows();
	if (selected_rows.isEmpty())
		return;

	m_mgr->kick_client_at_row(selected_rows[0].row());
}



void NetworkWidget::on_action_call()
{
	auto selected_rows = ui.connections_view->selectionModel()->selectedRows();
	if (selected_rows.isEmpty())
		return;

	m_mgr->call_client_at_row(selected_rows[0].row());
}


 
void NetworkWidget::on_context_menu_requested(const QPoint& pos)
{
	m_cursor_pos = pos;
	m_context_menu->exec(ui.connections_view->mapToGlobal(pos));
}



void NetworkWidget::on_list_view_double_click(const QModelIndex& index)
{
    m_mgr->call_client_at_row(index.row());
}