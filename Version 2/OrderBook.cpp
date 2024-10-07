#include "OrderBook.h"
#include <algorithm>
#include <thread>
#include <sstream>  // Include this for stringstream

void Insert_Order_into_OB(vector<Order> &Order_Book_Arg, Order New_Order, int Side) {
    if (Order_Book_Arg.empty()) {
        Order_Book_Arg.push_back(New_Order);
        return;
    }

    auto it = begin(Order_Book_Arg);
    while (it != end(Order_Book_Arg)) {
        if ((it->Price < New_Order.Price && Side == 1) || (it->Price > New_Order.Price && Side == 2)) {
            break;
        }
        ++it;
    }

    if (it == end(Order_Book_Arg)) {
        Order_Book_Arg.push_back(New_Order);
    } else {
        if ((it != begin(Order_Book_Arg)) && (it - 1)->Price == New_Order.Price) {
            New_Order.Order_Priority = ((it - 1)->Order_Priority) + 1;
        }
        Order_Book_Arg.insert(it, New_Order);
    }
}

void Process_Orders_and_Update_Books(string Input_File_Name) {

    // Create order books for 5 instruments (each with separate buy and sell books)
    vector<vector<Order>> Order_Book_Rose(2);
    vector<vector<Order>> Order_Book_Lavender(2);
    vector<vector<Order>> Order_Book_Lotus(2);
    vector<vector<Order>> Order_Book_Tulip(2);
    vector<vector<Order>> Order_Book_Orchid(2);

    // Store order books in an array
    vector<vector<Order>> Order_Book_Library[5] = {Order_Book_Rose, Order_Book_Lavender, Order_Book_Lotus, Order_Book_Tulip, Order_Book_Orchid};

    // Valid instrument names
    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    // Variables for reading input file
    vector<string> row;
    string line, word;

    // Set up input and output streams
    ifstream F_in(Input_File_Name);
    ofstream F_out("execution_rep.csv", ios::out);
    F_out << "Order ID,Client Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time\n";

    int Count = 0;

    // Process each line of the input file
    while (getline(F_in, line)) {
        if (++Count == 1) continue;  // Skip header

        row.clear();
        stringstream s(line);

        // Extract comma-separated values
        while (getline(s, word, ',')) {
            row.push_back(word);
        }

        // Create a new order from the input data
        Order New_Order(&row[0], &row[1], &row[2], &row[3], &row[4]);

        // Check if the order is not rejected
        if (New_Order.Check_If_Not_Rejected()) {
            // Find the corresponding order book for the instrument
            int Order_Book_Index = (int)(find(begin(Instrument_List), end(Instrument_List), New_Order.Instrument_Name) - begin(Instrument_List));
            vector<vector<Order>> Order_Book = Order_Book_Library[Order_Book_Index];

            // Process buy orders (Side = 1)
            if (New_Order.Side == 1) {
                // Match buy orders with sell orders if prices are favorable
                while (!(Order_Book[1].empty()) && (Order_Book[1][0].Price <= New_Order.Price)) {
                    if (New_Order.Remaining_Quantity == Order_Book[1][0].Remaining_Quantity) {
                        // Fully execute both orders
                        New_Order.Status = 2;
                        Order_Book[1][0].Status = 2;
                        New_Order.Execute(F_out);
                        Order_Book[1][0].Execute(F_out);
                        Order_Book[1].erase(begin(Order_Book[1]));
                        break;
                    } else if (New_Order.Remaining_Quantity > Order_Book[1][0].Remaining_Quantity) {
                        // Partially execute buy order, fully execute sell order
                        double temp = New_Order.Price;
                        New_Order.Status = 3;
                        Order_Book[1][0].Status = 2;
                        New_Order.Price = Order_Book[1][0].Price;
                        New_Order.Execute(F_out, Order_Book[1][0].Remaining_Quantity);
                        New_Order.Remaining_Quantity -= Order_Book[1][0].Remaining_Quantity;
                        Order_Book[1][0].Execute(F_out);
                        Order_Book[1].erase(begin(Order_Book[1]));
                        New_Order.Price = temp;
                    } else {
                        // Fully execute buy order, partially execute sell order
                        New_Order.Status = 2;
                        Order_Book[1][0].Status = 3;
                        New_Order.Price = Order_Book[1][0].Price;
                        New_Order.Execute(F_out);
                        Order_Book[1][0].Execute(F_out, New_Order.Remaining_Quantity);
                        Order_Book[1][0].Remaining_Quantity -= New_Order.Remaining_Quantity;
                        break;
                    }
                }

                // Insert remaining buy order into the book if partially or not executed
                if (New_Order.Status == 0) New_Order.Execute(F_out);
                if (New_Order.Remaining_Quantity > 0.0) Insert_Order_into_OB(Order_Book[0], New_Order, 1);

            } else if (New_Order.Side == 2) { // Process sell orders (Side = 2)
                // Match sell orders with buy orders if prices are favorable
                while (!(Order_Book[0].empty()) && (Order_Book[0][0].Price >= New_Order.Price)) {
                    if (New_Order.Remaining_Quantity == Order_Book[0][0].Remaining_Quantity) {
                        // Fully execute both orders
                        New_Order.Status = 2;
                        Order_Book[0][0].Status = 2;
                        New_Order.Price = Order_Book[0][0].Price;
                        New_Order.Execute(F_out);
                        Order_Book[0][0].Execute(F_out);
                        Order_Book[0].erase(begin(Order_Book[0]));
                        break;
                    } else if (New_Order.Remaining_Quantity > Order_Book[0][0].Remaining_Quantity) {
                        // Partially execute sell order, fully execute buy order
                        double temp2 = New_Order.Price;
                        New_Order.Status = 3;
                        Order_Book[0][0].Status = 2;
                        New_Order.Price = Order_Book[0][0].Price;
                        New_Order.Execute(F_out, Order_Book[0][0].Remaining_Quantity);
                        New_Order.Remaining_Quantity -= Order_Book[0][0].Remaining_Quantity;
                        Order_Book[0][0].Execute(F_out);
                        Order_Book[0].erase(begin(Order_Book[0]));
                        New_Order.Price = temp2;
                    } else {
                        // Fully execute sell order, partially execute buy order
                        New_Order.Status = 2;
                        Order_Book[0][0].Status = 3;
                        New_Order.Price = Order_Book[0][0].Price;
                        New_Order.Execute(F_out);
                        Order_Book[0][0].Execute(F_out, New_Order.Remaining_Quantity);
                        Order_Book[0][0].Remaining_Quantity -= New_Order.Remaining_Quantity;
                        break;
                    }
                }

                // Insert remaining sell order into the book if partially or not executed
                if (New_Order.Status == 0) New_Order.Execute(F_out);
                if (New_Order.Remaining_Quantity > 0.0) Insert_Order_into_OB(Order_Book[1], New_Order, 2);
            }

            // Update the order book after processing
            Order_Book_Library[Order_Book_Index] = Order_Book;

        } else {
            // If the order is rejected, log it as a reject
            New_Order.Execute(F_out);
        }
    }

    // Close input and output file streams
    F_in.close();
    F_out.close();
}
