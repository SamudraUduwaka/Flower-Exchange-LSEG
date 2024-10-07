#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "Order.h"
#include <vector>
#include <fstream>

void Insert_Order_into_OB(vector<Order>&, Order, int);
void Process_Orders_and_Update_Books(string Input_File_Name);

#endif
