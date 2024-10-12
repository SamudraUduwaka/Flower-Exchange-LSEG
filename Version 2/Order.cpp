#include "Order.h"
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

int Order::Current_Order_ID = 1;

Order::Order() {}

Order::Order(string* _Client_Order_ID, string* _Instrument_Name, string* _Side, string* _Price, string* _Quantity) {
    Client_Order_ID = *_Client_Order_ID;
    Order_ID = "ord" + to_string(Current_Order_ID++);
    Instrument_Name = *_Instrument_Name;
    Side = stoi(*_Side);
    Price = stod(*_Price);
    Quantity = stoi(*_Quantity);
    Check_Order_Validity(_Client_Order_ID, _Instrument_Name, _Side, _Quantity, _Price);
    Remaining_Quantity = Quantity;
    Order_Priority = 1;
}

void Order::Check_Order_Validity(string* _Client_Order_ID, string* _Instrument_Name, string* _Side, string* _Quantity, string* _Price) {
    if (!((*_Client_Order_ID).length() && (*_Instrument_Name).length() && (*_Side).length() && (*_Price).length() && (*_Quantity).length())) {
        Status = 1;
        Reason = "Invalid Fields";
        return;
    }

    if (find(begin(Instrument_List), end(Instrument_List), *_Instrument_Name) == end(Instrument_List)) {
        Status = 1;
        Reason = "Invalid Instrument";
        return;
    }

    int s = stoi(*_Side);
    if (!(s == 1) && !(s == 2)) {
        Status = 1;
        Reason = "Invalid Side";
        return;
    }

    double p = stod(*_Price);
    if (!(p > 0.0)) {
        Status = 1;
        Reason = "Invalid Price";
        return;
    }

    int q = stoi(*_Quantity);
    if (q % 10 != 0 || q < 10 || q > 1000) {
        Status = 1;
        Reason = "Invalid Size";
        return;
    }

    Status = 0;
    Reason = "Accepted";
}

bool Order::Check_If_Not_Rejected() {
    return Status != 1;
}

void Order::Execute(ofstream &F_out, int Exec_Quantity) {
    auto Current_Time = chrono::system_clock::now();
    auto Current_Time_in_Milliseconds = chrono::time_point_cast<chrono::milliseconds>(Current_Time);
    auto Milliseconds_since_Epoch = Current_Time_in_Milliseconds.time_since_epoch();
    long Milliseconds_Count = Milliseconds_since_Epoch.count();
    time_t Current_Time_as_a_time_t = chrono::system_clock::to_time_t(Current_Time);
    tm *Local_Time_Struct = localtime(&Current_Time_as_a_time_t);

    string Status_String = (Status == 0) ? "New" : (Status == 1) ? "Reject" : (Status == 2) ? "Fill" : "PFill";

    F_out << Order_ID << "," << Client_Order_ID << "," << Instrument_Name << "," 
          << Side << "," << Status_String << "," << Exec_Quantity << "," << Price
          << "," << Reason << "," << put_time(Local_Time_Struct, "%Y.%m.%d-%H.%M.%S") 
          << "." << setfill('0') << setw(3) << Milliseconds_Count % 1000 << "\n";
}

void Order::Execute(ofstream &F_out) {
    Execute(F_out, Remaining_Quantity);
}

