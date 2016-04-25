#ifndef _MENU_DATABASE_MODEL_H_
#define _MENU_DATABASE_MODEL_H_

#include "include/menudatabase.h"
#include <qabstractitemmodel.h>

class TreeNode {
public:
	~TreeNode() {
		for (auto& i : children) {
			delete i->data;
			delete i;
		}
		delete data;
	}

	MenuItem* data;
	TreeNode* parent;
	QVector<TreeNode*> children;
};


// это модель для класса MenuDatabase
class MenuDatabaseModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	MenuDatabaseModel(QSqlDatabase db);
	~MenuDatabaseModel();
	QVector<const MenuItem*> query_items() const;
	bool contains(int id) const { return m_items.contains(id); }
	MenuItem item(int id) const;
	QModelIndex add_category();
	QModelIndex add_menu_item(const QModelIndex& category);
	void remove_menu_item(const QModelIndex& index);
	bool set_image(const QModelIndex& index, const QImage& img);

	// Model interface
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orient, int role) const override;
	QModelIndex index(int row, int col, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = 2) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
	// tree interface
	int m_tree_add_item(TreeNode* category_node, MenuItem* item);
	void m_tree_delete_item(TreeNode* node);

	// some functions
	bool m_does_category_exist(const QString& category);
	TreeNode* m_get_category(const QString& category);

signals:
	void menu_changed();

private:
	MenuDatabase m_menu_db;
	TreeNode* m_root;
	QMap<int, TreeNode*> m_items;
	int free_id;
};

#endif