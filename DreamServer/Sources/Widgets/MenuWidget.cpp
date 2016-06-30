#include "include/widgets/MenuWidget.h"
#include "include/MenuItemDelegate.h"
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qmenu.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qmessagebox.h>
#include <assert.h>

MenuWidget::MenuWidget(MenuDatabaseModel* menu, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	assert(menu != nullptr);

	m_menu = menu;
	ui.tree_view->setModel(m_menu);
	ui.tree_view->setItemDelegate(new MenuItemDelegate(ui.tree_view));
	ui.tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.tree_view->setEditTriggers(QAbstractItemView::DoubleClicked 
		| QAbstractItemView::AnyKeyPressed);

	create_actions();

	m_category_menu = new QMenu();
	m_category_menu->addAction(m_remove_category_action);
	m_category_menu->addAction(m_add_item_action);
	
	m_item_menu = new QMenu();
	m_item_menu->addAction(m_remove_item_action);
	m_item_menu->addAction(m_change_img_action);
	m_item_menu->addAction(m_remove_img_action);

	m_tree_menu = new QMenu();
	m_tree_menu->addAction(m_add_category_action);

	connect(ui.tree_view, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(on_context_menu_requested(const QPoint&)));
}



MenuWidget::~MenuWidget()
{
	delete m_category_menu;
	delete m_item_menu;
	delete m_tree_menu;
	delete m_menu;
}



void MenuWidget::create_actions()
{
	m_add_category_action = new QAction(this);
	m_add_category_action->setText(QString::fromLocal8Bit("Добавить категорию"));
	connect(m_add_category_action, SIGNAL(triggered()),
		this, SLOT(on_add_category_action()));

	m_remove_category_action = new QAction(this);
	m_remove_category_action->setText(QString::fromLocal8Bit("Удалить категорию"));
	connect(m_remove_category_action, SIGNAL(triggered()),
		this, SLOT(on_remove_category_action()));

	m_add_item_action = new QAction(this);
	m_add_item_action->setText(QString::fromLocal8Bit("Добавить блюдо"));
	connect(m_add_item_action, SIGNAL(triggered()),
		this, SLOT(on_add_item_action()));

	m_remove_item_action = new QAction(this);
	m_remove_item_action->setText(QString::fromLocal8Bit("Удалить блюдо"));
	connect(m_remove_item_action, SIGNAL(triggered()),
		this, SLOT(on_remove_item_action()));

	m_change_img_action = new QAction(this);
	m_change_img_action->setText(QString::fromLocal8Bit("Изменить изображение"));
	connect(m_change_img_action, SIGNAL(triggered()),
		this, SLOT(on_change_img_action()));

	m_remove_img_action = new QAction(this);
	m_remove_img_action->setText(QString::fromLocal8Bit("Удалить изображение"));
	connect(m_remove_img_action, SIGNAL(triggered()),
		this, SLOT(on_remove_img_action()));
}





void MenuWidget::on_context_menu_requested(const QPoint& p)
{
	QPoint glob_p = ui.tree_view->mapToGlobal(p);
	QModelIndex index_at = ui.tree_view->indexAt(p);
	TreeNode* node = static_cast<TreeNode*>(index_at.internalPointer());

	if (!index_at.isValid()) {
		m_tree_menu->exec(glob_p);
	}
	else if (node->data->id == -1) {
		m_category_menu->exec(glob_p);
	}
	else {
		m_item_menu->exec(glob_p);
	}
}



void MenuWidget::on_add_category_action()
{
	m_menu->add_category();
	QModelIndex index = m_menu->index(m_menu->rowCount() - 1, 0, QModelIndex());
	ui.tree_view->setCurrentIndex(index);
	ui.tree_view->edit(index);
}



void MenuWidget::on_remove_category_action()
{
	auto selected_items = ui.tree_view->selectionModel()->selectedRows();

	if (selected_items.size() == 0)
		return;

	if (m_menu->hasChildren(selected_items.back())) {
		if (QMessageBox::warning(this, QString::fromLocal8Bit("Внимание"),
			QString::fromLocal8Bit("Категория не пуста.\nВы действительно хотите удалить ее?"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
			return;
	}

	m_menu->remove_menu_item(selected_items.back());
}



void MenuWidget::on_add_item_action()
{
	auto selected_items = ui.tree_view->selectionModel()->selectedRows();

	if (selected_items.size() == 0)
		return;

	QModelIndex index = m_menu->add_menu_item(selected_items.back());
	if (index.isValid()) {
		ui.tree_view->setCurrentIndex(index);
		ui.tree_view->edit(index);
	}
}



void MenuWidget::on_remove_item_action()
{
	auto selected_items = ui.tree_view->selectionModel()->selectedRows();

	if (selected_items.size() == 0)
		return;

	TreeNode* node = static_cast<TreeNode*>(selected_items.back().internalPointer());
	if (node->data->id == -1)
		return;

	m_menu->remove_menu_item(selected_items.back());
}



void MenuWidget::on_change_img_action()
{
    QSettings settings("dpiki", "dreamserver");
    QString curr_dir = settings.value("MenuWidget/curr_dir", QVariant("C:\\")).toString();
    QFileDialog dlg(this);
    dlg.setNameFilter(QString::fromLocal8Bit("Файлы изображений (*.png *.jpg *.jpeg *.bmp)"));
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setWindowTitle(QString::fromLocal8Bit("Выбор изображения"));
    dlg.setDirectory(curr_dir);

    QString img_name;
    if (dlg.exec()) {
        img_name = dlg.selectedFiles().back();
    }

    settings.setValue("MenuWidget/curr_dir", dlg.directory().absolutePath());

	QImage img(img_name);
	if (img.isNull())
		return;

	auto selected_items = ui.tree_view->selectionModel()->selectedRows();

	if (selected_items.size() == 0)
		return;

	m_menu->set_image(selected_items.back(), img);
}



void MenuWidget::on_remove_img_action()
{
 	auto selected_items = ui.tree_view->selectionModel()->selectedRows();

	if (selected_items.size() == 0)
		return;

	m_menu->set_image(selected_items.back(), QImage());
}