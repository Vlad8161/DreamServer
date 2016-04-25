#ifndef _ORDER_H_
#define _ORDER_H_

#include <qstring.h>

// объект заказа
class Order
{
public:
	const int n_fields = 8;

	enum Status {
		NOT_PREPARED,
		PREPARED,
		SERVED
	};

	enum Fields {
		Id = 0,
		Name = 1,
		Count = 2,
		TableNum = 3,
		TimeGot = 4,
		TimePrepared = 5,
		Stat = 6,
		Notes = 7
	};

	int id;
	QString name;
	int count;
	int table_num;
	QString time_got;
	QString time_prepared;
	int status;
	QString notes;
	
	Order();
	Order& operator=(const Order& ord1);
	friend bool operator==(const Order& ord1, const Order& ord2);
};


bool operator==(const Order& ord1, const Order& ord2);


#endif