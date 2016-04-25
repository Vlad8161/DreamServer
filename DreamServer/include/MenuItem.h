#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include <qimage.h>

// этот класс представляет собой элемент меню
class MenuItem
{
public:
	enum MenuItemFields {
		Id = 0,
		Name = 1,
		Category = 2,
		Image = 3
	};

	MenuItem() {}
	MenuItem(const MenuItem&);
	MenuItem& operator=(const MenuItem&);

	int id;
	QString name;
	QString category;
	QImage img;
};



inline MenuItem::MenuItem(const MenuItem& mi)
{
	id = mi.id;
	name = mi.name;
	category = mi.category;
	img = mi.img;
}



inline MenuItem& MenuItem::operator=(const MenuItem& mi)
{
	id = mi.id;
	name = mi.name;
	img = mi.img;
	category = mi.category;

	return *this;
}

#endif