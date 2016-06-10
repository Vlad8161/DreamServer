#ifndef _NETWORK_WIDGET_H_
#define _NETWORK_WIDGET_H_

#include "ui_network_widget.h"
#include "include/networkmanager.h"

/* Takes ownership of NetworkManager and its model
 * Provides interface to NetworkManager for other users
 * Viewes status infromation about NetworkManager
 */
class NetworkWidget : public QWidget
{
	Q_OBJECT

public:
	NetworkWidget(NetworkManager* network_manager, QWidget* parent = 0);
	~NetworkWidget();
	QList<QAction*> actions() const { return QList<QAction*>({ m_action_server_start, m_action_server_stop }); }

public slots:
	void on_action_server_start();
	void on_action_server_stop();
	void on_action_kick();
	void on_context_menu_requested(const QPoint& pos);
    void on_combo_box_index_changed(const QString&);
    void on_server_started();
    void on_server_stopped();

private:
	Ui::network_widget ui;
	NetworkManager* m_mgr;
	NetworkConnectionsModel* m_model;
	QAction* m_action_server_start;
	QAction* m_action_server_stop;
	QAction* m_action_kick;
	QMenu* m_context_menu;
	QPoint m_cursor_pos;
};

#endif