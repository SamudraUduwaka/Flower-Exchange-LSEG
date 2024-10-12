#include "Order.h"
#include <sstream>
#include <ctime>
#include <algorithm>
#include <chrono>

const vector<string> Order::instrument_list = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

Order::Order() : status(1), order_priority(1), remaining_quantity(0) {}

Order::Order(const string& client_id, const string& instrument, const string& side, const string& quantity, const string& price) {
    client_order_id = client_id;
    order_id = GenerateOrderId();
    instrument_name = instrument;
    order_side = ParseOrderSide(side);
    order_price = ParsePrice(price);
    order_quantity = ParseQuantity(quantity);
    
    ValidateOrder(client_id, instrument, side, quantity, price);

    remaining_quantity = order_quantity;
    order_priority = 1;
}

// Copy constructor
Order::Order(const Order& other)
    : client_order_id(other.client_order_id),
      instrument_name(other.instrument_name),
      order_side(other.order_side),
      order_price(other.order_price),
      order_quantity(other.order_quantity),
      order_id(other.order_id),
      status(other.status),
      reason(other.reason),
      remaining_quantity(other.remaining_quantity),
      order_priority(other.order_priority) {}

// Copy assignment operator
Order& Order::operator=(const Order& other) {
    if (this != &other) {
        client_order_id = other.client_order_id;
        instrument_name = other.instrument_name;
        order_side = other.order_side;
        order_price = other.order_price;
        order_quantity = other.order_quantity;
        order_id = other.order_id;
        status = other.status;
        reason = other.reason;
        remaining_quantity = other.remaining_quantity;
        order_priority = other.order_priority;
    }
    return *this;
}

// Move constructor
Order::Order(Order&& other) noexcept 
    : client_order_id(move(other.client_order_id)),
      instrument_name(move(other.instrument_name)),
      order_side(other.order_side),
      order_price(other.order_price),
      order_quantity(other.order_quantity),
      order_id(move(other.order_id)),
      status(other.status),
      reason(move(other.reason)),
      remaining_quantity(other.remaining_quantity),
      order_priority(other.order_priority) {}

// Move assignment operator
Order& Order::operator=(Order&& other) noexcept {
    if (this != &other) {
        client_order_id = move(other.client_order_id);
        instrument_name = move(other.instrument_name);
        order_side = other.order_side;
        order_price = other.order_price;
        order_quantity = other.order_quantity;
        order_id = move(other.order_id);
        status = other.status;
        reason = move(other.reason);
        remaining_quantity = other.remaining_quantity;
        order_priority = other.order_priority;
    }
    return *this;
}

void Order::ValidateOrder(const string& client_id, const string& instrument, const string& side, const string& quantity, const string& price) {
    if (client_id.empty() || instrument.empty() || side.empty() || quantity.empty() || price.empty()) {
        status = 1;
        reason = "Invalid Fields";
        return;
    }

    if (!IsInstrumentValid(instrument)) {
        status = 1;
        reason = "Invalid Instrument";
        return;
    }

    if (!(order_side == 1 || order_side == 2)) {
        status = 1;
        reason = "Invalid Side";
        return;
    }

    if (!(order_price > 0.0)) {
        status = 1;
        reason = "Invalid Price";
        return;
    }

    if (order_quantity % 10 != 0 || order_quantity < 10 || order_quantity > 1000) {
        status = 1;
        reason = "Invalid Size";
        return;
    }

    status = 0;
    reason = "Accepted";
}

bool Order::IsInstrumentValid(const string& instrument) {
    // Use std::find with vector
    return find(begin(instrument_list), end(instrument_list), instrument) != end(instrument_list);
}

int Order::ParseOrderSide(const string& side) {
    return stoi(side);
}

double Order::ParsePrice(const string& price) {
    return stod(price);
}

int Order::ParseQuantity(const string& quantity) {
    return stoi(quantity);
}

bool Order::IsNotRejected() const {
    return status != 1;
}

void Order::ExecuteOrder(ofstream& output_stream, int executed_quantity) {
    // Get the current time as a timestamp in milliseconds since the epoch
    auto now = chrono::system_clock::now();
    auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();

    string order_status = (status == 0) ? "New" : (status == 1) ? "Reject" : (status == 2) ? "Fill" : "Partial Fill";
    
    // Log order details along with the timestamp
    output_stream << order_id << "," << client_order_id << "," << instrument_name << "," 
                  << order_side << "," << order_status << "," << executed_quantity 
                  << "," << order_price << "," << reason << "," << timestamp << "\n";
}

void Order::ExecuteOrder(ofstream& output_stream) {
    ExecuteOrder(output_stream, remaining_quantity);
}

string Order::GenerateOrderId() {
    static int order_counter = 1;
    return "order_" + to_string(order_counter++);
}
