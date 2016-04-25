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

private:
	Ui::network_widget ui;
	NetworkManager* m_mgr;
	NetworkConnectionsModel* m_model;
	QAction* m_action_server_start;
	QAction* m_action_server_stop;
};

#endif