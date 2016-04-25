#include "include\widgets\dreamserver.h"
#include "include\ordersrepomodel.h"
#include "include\ordersrepoview.h"

#include <assert.h>

#include <qmessagebox.h>
#include <qevent.h>
#include <qdebug.h>
#include <qdockwidget.h>
#include <qtoolbar.h>
#include <qstackedlayout.h>
#include <qdesktopwidget.h>


ServerSolution::ServerSolution(QWidget *parent)
{
	ui.setupUi(this);
	
	QSqlDatabase orders_connection = QSqlDatabase::addDatabase("QSQLITE", "OrdersConnection");
	orders_connection.setDatabaseName("bar.db");
	orders_connection.open();
	QSqlDatabase menu_connection = QSqlDatabase::addDatabase("QSQLITE", "MenuConnection");
	menu_connection.setDatabaseName("bar.db");
	menu_connection.open();

	MenuDatabaseModel* menu = new MenuDatabaseModel(menu_connection);
	m_repo = new OrdersRepo(orders_connection, menu);
	NetworkManager* network_manager = new NetworkManager(menu, m_repo);
	m_menu_widget = new MenuWidget(menu);
	m_network_widget = new NetworkWidget(network_manager);
	m_network_dock = new QDockWidget(this);

	create_actions();
	create_tool_bar();
	create_central_widget();
	create_tray_icon();

	m_network_dock->setWidget(m_network_widget);
	m_network_dock->setWindowTitle(QString::fromLocal8Bit("Состояние сети"));
	addDockWidget(Qt::RightDockWidgetArea, m_network_dock);
	m_network_dock->close();

	connect(m_tab_widget, SIGNAL(tabCloseRequested(int)),
		this, SLOT(on_tab_close_requested(int)));
	connect(m_repo, SIGNAL(order_added(int, int)),
		this, SLOT(on_order_added(int)));
	connect(m_closed_table_widget, SIGNAL(undo_btn_clicked()),
		this, SLOT(on_undo_btn_clicked()));

	QRect screen_rect = QApplication::desktop()->screenGeometry();
	QRect top_widget_rect = screen_rect.marginsAdded(QMargins(-300, -150, -300, -150));
	this->setGeometry(top_widget_rect);
    
    m_network_widget->on_action_server_start();
}



ServerSolution::~ServerSolution()
{
	delete m_repo;
	delete m_menu_widget;
	delete m_network_widget;
	m_tray_icon->hide();
}



void ServerSolution::create_actions()
{

	m_action_network = m_network_dock->toggleViewAction();
	m_action_network->setIcon(QIcon(":/Resources/Icons/net.png"));

	m_action_show_menu = new QAction(this);
	m_action_show_menu->setText(QString::fromLocal8Bit("Меню"));
	m_action_show_menu->setIcon(QIcon(":/Resources/Icons/menu.png"));
	connect(m_action_show_menu, SIGNAL(triggered(bool)),
		this, SLOT(on_action_show_menu()));
}



void ServerSolution::create_tool_bar()
{
	m_tool_bar = addToolBar(QString::fromLocal8Bit("MainToolbar"));
	m_tool_bar->addActions(m_network_widget->actions());
	m_tool_bar->addSeparator();
	m_tool_bar->addAction(m_action_show_menu);
	m_tool_bar->addAction(m_action_network);
}



void ServerSolution::create_central_widget()
{
	m_tab_widget = new QTabWidget();
	m_tab_widget->setTabsClosable(true);
	m_closed_table_widget = new ClosedTableWidget(m_repo, m_tab_widget);
	m_closed_table_widget->hide();
	m_closed_table_widget->setFixedSize(250, 300);
	m_tab_widget->setMinimumHeight(m_closed_table_widget->height() + 20);
	m_tab_widget->setMinimumWidth(m_closed_table_widget->width() + 14);

	auto opened_tables = m_repo->opened_tables();
	for (auto i = opened_tables.begin(); i != opened_tables.end(); i++) {
		QWidget* w = m_open_table(*i);
		m_tab_widget->addTab(w, 
			QString::fromLocal8Bit("Столик №%1").arg(*i));
	}

	setCentralWidget(m_tab_widget);
}



void ServerSolution::create_tray_icon()
{
	m_tray_icon = new QSystemTrayIcon(QIcon(":/Resources/Icons/DreamIcon.png"), this);
	QMenu* tray_menu = new QMenu(this);

	m_action_quit = new QAction(tray_menu);
	m_action_quit->setText(QString::fromLocal8Bit("Выход"));
	connect(m_action_quit, SIGNAL(triggered()),
		this, SLOT(on_action_quit()));

	m_action_show_main_window = new QAction(tray_menu);
	m_action_show_main_window->setText("DreamServer");
	connect(m_action_show_main_window, SIGNAL(triggered()),
		this, SLOT(on_action_show_main_window()));

	tray_menu->addAction(m_action_show_main_window);
	tray_menu->addSeparator();
	tray_menu->addAction(m_action_quit);

	m_tray_icon->setContextMenu(tray_menu);
	m_tray_icon->setToolTip("DreamServer");
	connect(m_tray_icon, SIGNAL(messageClicked()),
		this, SLOT(on_action_show_main_window()));
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(on_tray_activated(QSystemTrayIcon::ActivationReason)));
	m_tray_icon->setVisible(true);
}



QWidget* ServerSolution::m_open_table(int t_num)
{
	OrdersRepoModel* model = m_repo->create_model(t_num);
	OrdersRepoView* view = new OrdersRepoView(model);
	m_opened_tables_nums.insert(t_num);
	return view;
}



void ServerSolution::on_tab_close_requested(int index)
{
	QString closing_title = m_tab_widget->tabBar()->tabText(index);
	int t_num = closing_title.section(QString::fromLocal8Bit("№"), 1, 1).toInt();
	auto orders = m_repo->query_table_orders(t_num);
	m_closed_table_widget->save_table_info(orders);
	if (m_repo->close_table(t_num)) {
		QWidget* w = m_tab_widget->widget(index);
		m_tab_widget->removeTab(index);
		m_opened_tables_nums.remove(t_num);
		m_closed_table_widget->show();
		delete w;	
	}
	else {
		QMessageBox::critical(this,
			QString::fromLocal8Bit("Ошибка закрытия стола"),
			QString::fromLocal8Bit("Невозможно закрыть стол №%1, так как он не обслужен!")
				.arg(t_num));
	}
}



void ServerSolution::on_order_added(int t_num)
{
	QString tab_title = QString::fromLocal8Bit("Столик №%1").arg(t_num);
	if (!m_opened_tables_nums.contains(t_num)) {
		QWidget* w = m_open_table(t_num);
		int new_tab_index = m_tab_widget->addTab(w, tab_title);
		m_tab_widget->show();
		m_tab_widget->setCurrentIndex(new_tab_index);
	}
	else {
		// Ищем столик в который был добавлен заказ
		int found = -1;
		auto tab_bar = m_tab_widget->tabBar();
		for (int i = 0; i < m_tab_widget->count(); i++) {
			if (m_tab_widget->tabBar()->tabText(i) == tab_title) {
				found = i;
				break;
			}
		}
		assert(found != -1);
		m_tab_widget->setCurrentIndex(found);
	}
	m_tray_icon->showMessage(
		QString::fromLocal8Bit("Новый заказ"),
		QString::fromLocal8Bit("Заказ добавлен на столик №%1").arg(t_num));
}



void ServerSolution::on_undo_btn_clicked()
{
	auto orders_to_restore = m_closed_table_widget->last_closed_table();

	if (orders_to_restore.size() == 0)
		return;

	m_repo->restore_orders(orders_to_restore);
	int t_num = orders_to_restore[0].table_num;
	if (!m_opened_tables_nums.contains(t_num)) {
		QWidget* w = m_open_table(t_num);
		int i_inserted_tab = m_tab_widget->insertTab(0, w, 
			QString::fromLocal8Bit("Столик №%1").arg(t_num));
		m_tab_widget->show();
		m_tab_widget->setCurrentIndex(i_inserted_tab);
	}

	m_closed_table_widget->hide();
}



void ServerSolution::on_tray_activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        this->show();
        this->activateWindow();
    }
}



void ServerSolution::on_action_show_menu()
{	
    m_menu_widget->exec();
}



void ServerSolution::on_action_show_main_window()
{
    this->activateWindow();
	this->show();
}



void ServerSolution::on_action_quit()
{
	QApplication::quit();
}



void ServerSolution::closeEvent(QCloseEvent* e)
{
	this->hide();
	e->ignore();
}



void ServerSolution::resizeEvent(QResizeEvent* e)
{
	int new_y = m_tab_widget->height() - m_closed_table_widget->height() - 7;
	m_closed_table_widget->move(7, new_y);
}