#include "OrderBook.h"
#include <sstream>
#include <thread>
#include <algorithm>

void InsertOrderToBook(vector<Order>& order_book, Order& new_order, int side) {
    auto position = order_book.begin();
    while (position != order_book.end()) {
        if ((side == 1 && position->order_price < new_order.order_price) ||
            (side == 2 && position->order_price > new_order.order_price)) {
            break;
        }
        ++position;
    }

    order_book.insert(position, move(new_order));  // Use move semantics here
}

void ProcessOrders(const string& input_file_name) {
    vector<vector<Order>> order_books[5];  // Declare array

    for (int i = 0; i < 5; ++i) {
        order_books[i].push_back(vector<Order>());  // Initialize dynamically
        order_books[i].push_back(vector<Order>());
    }

    const vector<string> instrument_list = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    ifstream input_file(input_file_name);
    ofstream output_file("execution_report.csv");
    output_file << "Order ID,Client Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time\n";

    string line;
    int line_count = 0;
    
    while (getline(input_file, line)) {
        if (++line_count == 1) continue;  // Skip header

        stringstream ss(line);
        vector<string> row(5);
        for (int i = 0; i < 5; ++i) {
            getline(ss, row[i], ',');
        }

        Order new_order(row[0], row[1], row[2], row[3], row[4]);

        // Only execute if the order is not rejected
        if (new_order.IsNotRejected()) {
            int book_index = find(begin(instrument_list), end(instrument_list), new_order.instrument_name) - begin(instrument_list);

            if (new_order.order_side == 1) {
                // Match buy orders with sell orders
                ExecuteOrders(order_books[book_index][1], new_order, output_file);
                InsertOrderToBook(order_books[book_index][0], new_order, 1);  // Insert into buy book
            } else {
                // Match sell orders with buy orders
                ExecuteOrders(order_books[book_index][0], new_order, output_file);
                InsertOrderToBook(order_books[book_index][1], new_order, 2);  // Insert into sell book
            }

            // Only log the order as "New" if it hasn't been fully or partially matched
            if (new_order.status == 0) {
                new_order.ExecuteOrder(output_file);  // Log the "New" order if no match was found
            }
        } else {
            // If rejected, log it as rejected
            new_order.ExecuteOrder(output_file);
        }
    }

    input_file.close();
    output_file.close();
}


void ExecuteOrders(vector<Order>& opposite_orders, Order& new_order, ofstream& output_file) {
    while (!opposite_orders.empty()) {
        Order& best_opposite_order = opposite_orders.front();

        if ((new_order.order_side == 1 && best_opposite_order.order_price > new_order.order_price) ||
            (new_order.order_side == 2 && best_opposite_order.order_price < new_order.order_price)) {
            break;
        }

        if (new_order.remaining_quantity == best_opposite_order.remaining_quantity) {
            new_order.status = 2;  // Fully matched
            best_opposite_order.status = 2;

            new_order.ExecuteOrder(output_file);  // Log the fully matched order
            best_opposite_order.ExecuteOrder(output_file);

            opposite_orders.erase(opposite_orders.begin());
            break;
        } else if (new_order.remaining_quantity > best_opposite_order.remaining_quantity) {
            new_order.status = 3;  // Partially filled
            best_opposite_order.status = 2;

            new_order.ExecuteOrder(output_file, best_opposite_order.remaining_quantity);
            new_order.remaining_quantity -= best_opposite_order.remaining_quantity;

            best_opposite_order.ExecuteOrder(output_file);
            opposite_orders.erase(opposite_orders.begin());
        } else {
            new_order.status = 2;
            best_opposite_order.status = 3;

            new_order.ExecuteOrder(output_file);
            best_opposite_order.ExecuteOrder(output_file, new_order.remaining_quantity);

            best_opposite_order.remaining_quantity -= new_order.remaining_quantity;
            break;
        }
    }
}
