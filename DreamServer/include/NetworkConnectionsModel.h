#ifndef _NETWORK_CONNECTIONS_MODEL_H
#define _NETWORK_CONNECTIONS_MODEL_H

#include <qabstractitemmodel.h>
#include "include/networkmanager.h"

/* Model for NetworkManager that presents data about active connections
 */
class NetworkConnectionsModel : public QAbstractListModel
{
	Q_OBJECT

public:
	NetworkConnectionsModel(const NetworkManager* manager);
	~NetworkConnectionsModel();

	int rowCount(const QModelIndex& index = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;

public slots:
	void on_connection_list_changed();
	void on_client_status_changed(NetworkResponser* responser);

private:
	const NetworkManager* m_network_manager;
};

#endif