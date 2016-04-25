#include "include/menudatabasemodel.h"
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <assert.h>


MenuDatabaseModel::MenuDatabaseModel(QSqlDatabase db) : m_menu_db(db)
{
	QVector<MenuItem> items;

	bool ok = m_menu_db.query_items(items);
	assert(ok);
	
	m_root = new TreeNode();
	m_root->parent = nullptr;

	free_id = 0;
	for (auto& i : items) {
		MenuItem* item = new MenuItem(i);

		TreeNode* category_node = m_get_category(item->category);
		TreeNode* node = category_node->children[m_tree_add_item(category_node, item)];

		m_items[i.id] = node;
	}

	free_id = 0;
}



MenuDatabaseModel::~MenuDatabaseModel()
{
}



QVector<const MenuItem*> MenuDatabaseModel::query_items() const
{
	QVector<const MenuItem*> ret_val;
	for (auto& i : m_items)
		ret_val.push_back(i->data);

	return ret_val;
}



MenuItem MenuDatabaseModel::item(int id) const
{
	if (!m_items.contains(id))
		return MenuItem();
	else
		return *(m_items[id]->data);
}



QModelIndex MenuDatabaseModel::add_menu_item(
	const QModelIndex& category)
{
	while (m_items.contains(free_id))
		free_id++;

	TreeNode* category_node = static_cast<TreeNode*>(category.internalPointer());
	if (category_node->parent != m_root)
		return QModelIndex();

	MenuItem* item = new MenuItem();
	item->id = free_id;
	item->name = QString::fromLocal8Bit("Новое блюдо");
	item->category = category_node->data->name;

	if (!m_menu_db.add_menu_item(*item)) {
		delete item;
		assert(false);
		return QModelIndex();
	}

	beginResetModel();

	int i_node = m_tree_add_item(category_node, item);
	m_items[item->id] = category_node->children[i_node];
	free_id++;

	endResetModel();

	emit menu_changed();

	return createIndex(i_node, 0, category_node->children[i_node]);
}



QModelIndex MenuDatabaseModel::add_category()
{
	MenuItem* new_item;
	TreeNode* new_node;
	QString new_category_name = QString::fromLocal8Bit("Новая категория 0");

	bool is_exist;
	do {
		int i = new_category_name.mid(16).toInt() + 1;
		new_category_name = QString::fromLocal8Bit("Новая категория %1").arg(i);
	} while (m_does_category_exist(new_category_name));

	beginResetModel();

	TreeNode* node = m_get_category(new_category_name);

	endResetModel();

	return createIndex(m_root->children.indexOf(node), 0, node);
}



void MenuDatabaseModel::remove_menu_item(const QModelIndex& index)
{
	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
	MenuItem* item = node->data;

	if (item->id != -1) {
		// category
		bool ok = m_menu_db.remove_menu_item(item->id);
		assert(ok);
	}
	else {
		bool ok = m_menu_db.remove_category(item->name);
		assert(ok);
	}

	beginResetModel();

	m_tree_delete_item(node);
	m_items.remove(item->id);

	emit menu_changed();

	endResetModel();
}



bool MenuDatabaseModel::set_image(const QModelIndex& index, const QImage& img)
{
	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());

	if (!m_menu_db.change_img(node->data->id, img))
		return false;

	node->data->img = img;

	emit dataChanged(index, index, QVector<int>(1, Qt::DecorationRole));

	emit menu_changed();

	return true;
}



QVariant MenuDatabaseModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
	MenuItem* item = node->data;

	if (role == Qt::DisplayRole) {
		if (index.column() == 0) {
			return QVariant(item->name);
		}
	}

	if (role == Qt::DecorationRole) {
		if (index.column() == 0) {
			return QVariant(item->img.scaled(30, 30));
		}
	}

	if (role == Qt::SizeHintRole) {
		if (index.column() == 0) {
			return QVariant(QSize(2000, 30));
		}
	}

	return QVariant();
}



bool MenuDatabaseModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid()) {
		return false;
	}

	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
	MenuItem* item = node->data;

	if (node->parent == nullptr)
		return false;

	if (node->parent == m_root) {
		// категория
		QString new_category = value.toString();

		if (new_category.isEmpty() || m_does_category_exist(new_category))
			return false;

		bool ok = m_menu_db.rename_category(item->name, new_category);
		assert(ok);

		item->name = new_category;
		for (auto& i : node->children)
			i->data->category = new_category;
	}
	else {
		// элемент меню
		QString new_name = value.toString();
		if (new_name.isEmpty())
			return false;

		bool ok = m_menu_db.change_name(item->id, new_name);
		assert(ok);

		item->name = new_name;
	}

	emit dataChanged(index, index, QVector<int>(1, Qt::DisplayRole));

	emit menu_changed();

	return true;
}



QVariant MenuDatabaseModel::headerData(int section, Qt::Orientation o, int role) const
{
	if (role == Qt::DisplayRole && o == Qt::Horizontal) {
		switch (section)
		{
		case 0:
			return QVariant(QString::fromLocal8Bit("Название"));
		default:
			break;
		}
	}

	return QVariant();
}



Qt::ItemFlags MenuDatabaseModel::flags(const QModelIndex& index) const
{
	if (index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	else 
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}



int MenuDatabaseModel::rowCount(const QModelIndex& parent) const
{
	if (parent == QModelIndex())
		return m_root->children.size();

	TreeNode* node;
	if (parent == QModelIndex())
		node = m_root;
	else
		node = static_cast<TreeNode*>(parent.internalPointer());

	return node->children.size();
}



QModelIndex MenuDatabaseModel::index(int row, int col, const QModelIndex& parent) const
{
	if (col != 0)
		return QModelIndex();

	TreeNode* tree_parent;
	if (parent == QModelIndex())
		tree_parent = m_root;
	else
		tree_parent = static_cast<TreeNode*>(parent.internalPointer());

	if (row >= tree_parent->children.size())
		return QModelIndex();
	else
		return createIndex(row, col, tree_parent->children[row]);
}



QModelIndex MenuDatabaseModel::parent(const QModelIndex& index) const
{
	if (index == QModelIndex())
		return QModelIndex();

	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
	TreeNode* parent = node->parent;
	TreeNode* grand_parent = parent->parent;

	if (parent == m_root)
		return QModelIndex();

	int parent_row = grand_parent->children.indexOf(parent);
	assert(parent_row != -1);

	return createIndex(parent_row, 0, parent);
}



int MenuDatabaseModel::m_tree_add_item(TreeNode* category_node, MenuItem* item)
{
	QString category;

	TreeNode* new_node = new TreeNode();
	new_node->data = item;
	new_node->parent = category_node;
	category_node->children.append(new_node);

	return category_node->children.indexOf(new_node);
}



void MenuDatabaseModel::m_tree_delete_item(TreeNode* node)
{
	TreeNode* parent = node->parent;

	int i_child = parent->children.indexOf(node);
	assert(i_child != -1);

	parent->children.remove(i_child);
}



bool MenuDatabaseModel::m_does_category_exist(const QString& category)
{
	for (auto& i : m_root->children) {
		if (i->data->name == category)
			return true;
	}

	return false;
}



TreeNode* MenuDatabaseModel::m_get_category(const QString& category)
{
	TreeNode* found = nullptr;
	for (auto& i : m_root->children) {
		if (i->data->name == category) {
			found = i;
			break;
		}
	}

	if (found == nullptr) {
		found = new TreeNode();
		found->data = new MenuItem();
		found->data->id = -1;
		found->data->name = category;
		found->parent = m_root;
		m_root->children.append(found);
	}

	return found;
}