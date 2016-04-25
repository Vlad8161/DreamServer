#include "include/networkconnectionsmodel.h"
#include <qtcpsocket.h>
#include <assert.h>



NetworkConnectionsModel::NetworkConnectionsModel(const NetworkManager* mgr)
	: m_network_manager(mgr)
{

}



NetworkConnectionsModel::~NetworkConnectionsModel()
{

}



int NetworkConnectionsModel::rowCount(const QModelIndex& index) const
{
	return m_network_manager->n_clients();
}



QVariant NetworkConnectionsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole) {
		auto clients = m_network_manager->clients();
		if (index.row() < clients.size()) {
			return QVariant(clients[index.row()]->name());
		}
		else {
			return QVariant();
		}
	}

	return QVariant();
}



void NetworkConnectionsModel::on_connection_list_changed()
{
	beginResetModel();
	endResetModel();
}



void NetworkConnectionsModel::on_client_status_changed(NetworkResponser* responser)
{
	auto clients = m_network_manager->clients();
	auto it = qFind(clients, responser);
	
	assert(it != clients.end());

	int i = it - clients.begin();

	auto index = this->createIndex(i, 0, nullptr);
	emit dataChanged(index, index);
}