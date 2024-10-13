#include "OrderBook.h"
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <future>  // For asynchronous threading

std::mutex output_mutex;  // Mutex to protect output file access

void InsertOrderToBook(std::vector<Order>& order_book, Order& new_order, int side) {
    auto position = order_book.begin();
    while (position != order_book.end()) {
        if ((side == 1 && position->order_price < new_order.order_price) ||
            (side == 2 && position->order_price > new_order.order_price)) {
            break;
        }
        ++position;
    }

    order_book.insert(position, std::move(new_order));  // Use move semantics to avoid unnecessary copying
}

void ProcessInstrumentOrders(std::vector<Order>& buy_orders, std::vector<Order>& sell_orders, 
                             const std::string& instrument, std::ofstream& output_file) {
    // Process buy and sell orders (thread-safe file write happens inside functions that handle execution)
    // Since we're focusing on speed, ensure only necessary synchronization
}

void ProcessOrders(const std::string& input_file_name) {
    std::vector<std::vector<Order>> order_books[5];  // Declare array for 5 instruments

    for (int i = 0; i < 5; ++i) {
        order_books[i].push_back(std::vector<Order>());  // Buy orders
        order_books[i].push_back(std::vector<Order>());  // Sell orders
    }

    const std::vector<std::string> instrument_list = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    std::ifstream input_file(input_file_name);
    std::ofstream output_file("execution_report.csv");
    output_file << "Order ID,Client Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time\n";

    std::string line;
    int line_count = 0;
    
    std::vector<std::future<void>> futures;  // To handle async tasks

    while (getline(input_file, line)) {
        if (++line_count == 1) continue;  // Skip header

        std::stringstream ss(line);
        std::vector<std::string> row(5);
        for (int i = 0; i < 5; ++i) {
            getline(ss, row[i], ',');
        }

        Order new_order(row[0], row[1], row[2], row[3], row[4]);

        if (new_order.IsNotRejected()) {
            {
                std::lock_guard<std::mutex> lock(output_mutex);  // Protect file output
                new_order.ExecuteOrder(output_file);  // Log the "New" order
            }

            int book_index = std::find(instrument_list.begin(), instrument_list.end(), new_order.instrument_name) - instrument_list.begin();

            if (new_order.order_side == 1) {
                futures.push_back(std::async(std::launch::async, ExecuteOrders, std::ref(order_books[book_index][1]), 
                                             std::ref(new_order), std::ref(output_file)));  // Match buy order with sell orders
                InsertOrderToBook(order_books[book_index][0], new_order, 1);
            } else {
                futures.push_back(std::async(std::launch::async, ExecuteOrders, std::ref(order_books[book_index][0]), 
                                             std::ref(new_order), std::ref(output_file)));  // Match sell order with buy orders
                InsertOrderToBook(order_books[book_index][1], new_order, 2);
            }
        } else {
            std::lock_guard<std::mutex> lock(output_mutex);
            new_order.ExecuteOrder(output_file);  // Log rejected orders
        }
    }

    input_file.close();

    // Wait for all async tasks to finish
    for (auto& future : futures) {
        future.get();
    }

    output_file.close();
}

void ExecuteOrders(std::vector<Order>& opposite_orders, Order& new_order, std::ofstream& output_file) {
    // First, attempt exact quantity matching
    auto exact_match = std::find_if(opposite_orders.begin(), opposite_orders.end(),
                                    [&new_order](const Order& order) {
                                        return order.remaining_quantity == new_order.remaining_quantity;
                                    });

    // If we found an exact match, execute it
    if (exact_match != opposite_orders.end()) {
        new_order.status = 2;  // Fully matched
        exact_match->status = 2;

        {
            std::lock_guard<std::mutex> lock(output_mutex);  // Protect file output
            new_order.ExecuteOrder(output_file);  // Log the fully matched order
            exact_match->ExecuteOrder(output_file);
        }

        opposite_orders.erase(exact_match);  // Remove the matched order
        return;  // Exit as the order is fully matched
    }

    // If no exact match was found, fall back to standard matching based on order
    while (!opposite_orders.empty()) {
        Order& best_opposite_order = opposite_orders.front();

        if ((new_order.order_side == 1 && best_opposite_order.order_price > new_order.order_price) ||
            (new_order.order_side == 2 && best_opposite_order.order_price < new_order.order_price)) {
            break;
        }

        if (new_order.remaining_quantity == best_opposite_order.remaining_quantity) {
            new_order.status = 2;  // Fully matched
            best_opposite_order.status = 2;

            {
                std::lock_guard<std::mutex> lock(output_mutex);  // Protect file output
                new_order.ExecuteOrder(output_file);  // Log the fully matched order
                best_opposite_order.ExecuteOrder(output_file);
            }

            opposite_orders.erase(opposite_orders.begin());
            break;
        } else if (new_order.remaining_quantity > best_opposite_order.remaining_quantity) {
            new_order.status = 3;  // Partially filled
            best_opposite_order.status = 2;

            {
                std::lock_guard<std::mutex> lock(output_mutex);  // Protect file output
                new_order.ExecuteOrder(output_file, best_opposite_order.remaining_quantity);
                best_opposite_order.ExecuteOrder(output_file);
            }

            new_order.remaining_quantity -= best_opposite_order.remaining_quantity;
            opposite_orders.erase(opposite_orders.begin());
        } else {
            new_order.status = 2;
            best_opposite_order.status = 3;

            {
                std::lock_guard<std::mutex> lock(output_mutex);  // Protect file output
                new_order.ExecuteOrder(output_file);
                best_opposite_order.ExecuteOrder(output_file, new_order.remaining_quantity);
            }

            best_opposite_order.remaining_quantity -= new_order.remaining_quantity;
            break;
        }
    }
}
