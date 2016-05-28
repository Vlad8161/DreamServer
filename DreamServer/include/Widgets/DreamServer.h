#ifndef SERVERSOLUTION_H
#define SERVERSOLUTION_H

#include <QtWidgets/QMainWindow>
#include <qsystemtrayicon.h>

#include "ui_dreamserver.h"
#include "include\OrdersRepo.h"
#include "include\MenuDatabase.h"
#include "include\NetworkManager.h"
#include "include\widgets\ClosedTableWidget.h"
#include "include\widgets\MenuWidget.h"
#include "include\widgets\NetworkWidget.h"

/* Top level widget that initializes other components
 */
class ServerSolution : public QMainWindow
{
	Q_OBJECT

public:
	ServerSolution(QWidget *parent = 0);
	~ServerSolution();

public slots:
	void on_tab_close_requested(int index);
	void on_order_added(int t_num);
	void on_undo_btn_clicked();
	void on_tray_activated(QSystemTrayIcon::ActivationReason reason);
	void on_action_quit();
	void on_action_show_main_window();

protected:
	void closeEvent(QCloseEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;

private:
	QWidget* m_open_table(int t_num);
	void create_actions();
	void create_tool_bar();
	void create_central_widget();
	void create_tray_icon();

	Ui::ServerSolutionClass ui;

	QTabWidget* m_tab_widget;
	QDockWidget* m_network_dock;
	QDockWidget* m_menu_dock;
	MenuWidget* m_menu_widget;
	ClosedTableWidget* m_closed_table_widget;
	NetworkWidget* m_network_widget;

	QSystemTrayIcon* m_tray_icon;
	QAction* m_action_quit;
	QAction* m_action_show_main_window;
	QToolBar* m_tool_bar;
	QAction* m_action_server_start;
	QAction* m_action_server_stop;
	QAction* m_action_show_menu;
	QAction* m_action_network;

	OrdersRepo* m_repo;

	QSet<int> m_opened_tables_nums;
};

#endif // SERVERSOLUTION_H