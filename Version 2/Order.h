#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

using namespace std;

class Order {
public:
    string Client_Order_ID;
    string Instrument_Name;
    int Side;
    double Price;
    int Quantity;
    string Order_ID;
    int Status;
    string Reason;
    int Remaining_Quantity;
    int Order_Priority;

    static int Current_Order_ID;
    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    Order();
    Order(string*, string*, string*, string*, string*);
    
    void Check_Order_Validity(string*, string*, string*, string*, string*);
    bool Check_If_Not_Rejected();
    void Execute(ofstream&, int);
    void Execute(ofstream&);

private:
    string getCurrentTime();
};

#endif
