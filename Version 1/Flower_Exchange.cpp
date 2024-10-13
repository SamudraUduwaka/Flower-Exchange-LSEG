/*----------------------------- LSEG Flower Exchange Project (Based on C++) -----------------------------*/



/*--------------------------- Including necessary libraries and header files ---------------------------*/

#include <iostream>

#include <fstream>

#include <string>
#include <sstream>

#include <chrono>
#include <ctime>
#include <iomanip> 

#include <algorithm> 

#include <vector> 

#include <iterator> 

using namespace std;




struct Order 
{


    /*------------------------------------- Main Attributes ----------------------------------------*/


    // Input file & Output file common attributes
    string Client_Order_ID;
    string Instrument_Name;
    int Side;
    double Price;
    int Quantity;


    // Output file specific attributes
    string Order_ID;
    int Status;
    string Reason;




    /*--------------------------------- Other Necessary Attributes --------------------------------*/


    // Represents the quantity of the order that is remaining to be executed.
    int Remaining_Quantity;


    // Represents the priority of the order within the order book.
    int Order_Priority;


    // List of valid instruments
    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};


    /*       Using "static" ensures that the Order ID is managed at the 
    struct level, and is shared among all instances of the Order struct.
    So every Order object will get a unique Order ID.                */
    static int Current_Order_ID;




    /*---------------------- Default Constructor & Parameterized Constructor ----------------------*/


    Order() 
    {}


    Order(string *_Client_Order_ID, string *_Instrument_Name, string *_Side, string *_Price, string *_Quantity) 
    {
        /*   Using pointers to receive data that already exists in memory and pass 
        it to the constructor without making unnecessary copies of the data.    */
        
        Client_Order_ID = *_Client_Order_ID;
        Order_ID = "ord" + to_string(Current_Order_ID++); 
        Instrument_Name = *_Instrument_Name;
        Side = stoi(*_Side);
        Price = stod(*_Price);
        Quantity = stoi(*_Quantity);

        // Validates order fields and sets Reason and Status attributes
        Check_Order_Validity(_Client_Order_ID, _Instrument_Name, _Side, _Quantity, _Price);

        // Initially, entire quantity of the order is remaining to be executed.
        Remaining_Quantity = Quantity; 

        // Initially, order priority is set to 1.
        Order_Priority = 1;

    }




    /*------------------------- Necessary Functions for the Order Struct --------------------------*/


    // Check if order is valid or not and set Status & Reason.
    void Check_Order_Validity(string *_Client_Order_ID, string *_Instrument_Name, string *_Side, string *_Quantity, string *_Price) 
    {

        // Check if any of the order fields are empty.
        // If empty, mark Status as 1 (Reject) and Reason as "Invalid Fields."
        if (!((*_Client_Order_ID).length() && (*_Instrument_Name).length() && (*_Side).length() && (*_Price).length() && (*_Quantity).length())) 
        {
            Status = 1;
            Reason = "Invalid Fields";
            return;
        }

        // Checks if provided instrument name in order matches any of the valid instrument names.
        // If not matching, mark Status as 1 (Reject) and Reason as "Invalid Instrument."
        if (find(begin(Instrument_List), end(Instrument_List), *_Instrument_Name) == end(Instrument_List)) 
        {
            Status = 1;
            Reason = "Invalid Instrument";
            return;
        }

        // Checks if provided side in order is either 1 (buy) or 2 (sell).
        // If neither 1 nor 2, mark Status as 1 (Reject) and Reason as "Invalid Side."
        int s = stoi(*_Side);
        if (!(s == 1) && !(s == 2)) 
        {
            Status = 1;
            Reason = "Invalid Side";
            return;
        }

        // Checks if provided price in order is a positive number.
        // If not positive, mark Status as 1 (Reject) and Reason as "Invalid Price."
        double p = stod(*_Price);
        if (!(p > 0.0)) 
        {
            Status = 1;
            Reason = "Invalid Price";
            return;
        }

        // Checks if provided order quantity is a multiple of 10 & within the range of 10 to 1000. 
        // If not, mark Status as 1 (Reject) and Reason as "Invalid Size."
        int q = stoi(*_Quantity);
        if (q % 10 != 0 || q < 10 || q > 1000) 
        {
            Status = 1;
            Reason = "Invalid Size";
            return;
        }

        // If all order fields are valid, mark Status as 0 (New) and Reason as "Accepted."
        Status = 0;
        Reason = "Accepted";

    }


    // Return a boolean value indicating whether order has been Reject or not. 
    bool Check_If_Not_Rejected() 
    {
        if (Status == 1)
        {
            return false;
        }
        return true;
    }


    // Generate and format order execution details and write them to the output file.
    // F_out is a reference to the output file stream. Exec_Quantity is the quantity of the order to be executed.
    void Execute(ofstream &F_out, int Exec_Quantity)  
    {

        // Retrieve current time as a time_point object using system_clock. 
        auto Current_Time = chrono::system_clock::now(); 

        // Convert time_point object to millisecond precison. We don't need more precision than that.
        auto Current_Time_in_Milliseconds = chrono::time_point_cast<chrono::milliseconds>(Current_Time);

        // Get the number of milliseconds since epoch and convert it to a long integer.
        auto Milliseconds_since_Epoch = Current_Time_in_Milliseconds.time_since_epoch();
        long Milliseconds_Count = Milliseconds_since_Epoch.count();

        // Convert the time_point object "Current_Time" into a time_t object.
        time_t Current_Time_as_a_time_t = chrono::system_clock::to_time_t(Current_Time);

        // Convert the time_t object "Current_Time_as_a_time_t" into a tm struct.
        // tm structure in C++ can hold various components of calendar time, such as years, months,....
        tm *Local_Time_Struct = localtime(&Current_Time_as_a_time_t);

        // Convert Status from int to String for writing to output file.
        string Status_String = "";
        if (Status == 0)
        {
            Status_String = "New";
        }
        else if (Status == 1)
        {
            Status_String = "Reject";
        }
        else if (Status == 2)
        {
            Status_String = "Fill";
        }
        else if (Status == 3)
        {
            Status_String = "PFill";
        }

        // Write order execution details to output file.
        F_out << Order_ID << "," << Client_Order_ID << ","  << Instrument_Name << "," 
        << Side << "," << Status_String << "," << Exec_Quantity << ","  << Price 
        << "," << Reason << "," << put_time(Local_Time_Struct, "%Y.%m.%d-%H.%M.%S") 
        << "." << setfill('0') << setw(3) << Milliseconds_Count % 1000 << "\n";
    
    }


    // Execute orders with remaining quantity and write the execution details to an output file. 
    // Recursively call the main Execute function with same output file stream & remaining order quantity.
    void Execute(ofstream &F_out) 
    {
        Execute(F_out, Remaining_Quantity);
    }


};




// Initialize the static variable Current_Order_ID to 1.
int Order::Current_Order_ID = 1;




void Insert_Order_into_OB(vector<Order> &Order_Book_Arg, Order New_Order, int Side) 
{

    // Check if the interested order book is empty. 
    // If empty, just append the new order to the order book and return.
    if (Order_Book_Arg.empty()) 
    {
        Order_Book_Arg.push_back(New_Order);
        return;
    }

    // Pointer to the first element of the order book.
    auto it = begin(Order_Book_Arg); 

    /*     Checks if the price of the current order in order book is less than the price of 
    the new order (for buy side) or greater than the price of the new order (for sell side). 
    If this condition is true, it means the correct position is found, so loop will break.     */
    while (it != end(Order_Book_Arg)) 
    {
        if ((it->Price < New_Order.Price && Side == 1) || (it->Price > New_Order.Price && Side == 2))
        {
            break;
        }
        ++it;
    }

    /*    Check if the iterator reached the end of the order book. If it did, that means 
    the new order should be added at the end of the order book using the .push_back method. 
    If the iterator did not reach the end, that means the new order should be inserted 
    at the current position indicated by the iterator using the .insert method. 
    Also, if the previous order in the order book has the same price as the new order, the order 
    priority of the new order is adjusted to be one more than the priority of the previous order.  */
    if (it == end(Order_Book_Arg)) 
    {
        Order_Book_Arg.push_back(New_Order);
    } 
    else 
    {
        if ((it != begin(Order_Book_Arg)) && (it - 1)->Price == New_Order.Price) 
        {
            New_Order.Order_Priority = ((it - 1)->Order_Priority) + 1;
        }
        Order_Book_Arg.insert(it, New_Order);
    }

}




void Process_Orders_and_Update_Books(string Input_File_Name)
{


    // Create 5 order books for 5 instruments.
    vector<vector<Order>> Order_Book_Rose(2);
    vector<vector<Order>> Order_Book_Lavender(2);
    vector<vector<Order>> Order_Book_Lotus(2);
    vector<vector<Order>> Order_Book_Tulip(2);
    vector<vector<Order>> Order_Book_Orchid(2);


    // Store the 5 order books in an array.
    vector<vector<Order>> Order_Book_Library[5] = {Order_Book_Rose, Order_Book_Lavender, Order_Book_Lotus, Order_Book_Tulip, Order_Book_Orchid};


    // List of valid instruments
    string Instrument_List[5] = {"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};

    
    vector<string> row;  // To store individual values in a row from the CSV file
    string line;         // To temporarily store each line read from the CSV file
    string word;         // To temporarily store each word (field) extracted from a line
    string temp;         // Temporary string used for processing


    // File handling & setting up input and output streams.
    ifstream F_in; 
    F_in.open(Input_File_Name, ios::in); 
    ofstream F_out;
    F_out.open("execution_rep.csv", ios::out);
    F_out << "Order ID,Client Order ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time\n";
    
    
    int Count = 0;               // To keep track of the number of lines read from the input file.


    while (getline(F_in, line))  // This while-loop will continue until there are no more lines left to read.
    {


        if (++Count == 1)        // This is done to ignore the header line.
        {
            continue;
        }

        row.clear();            // Clear the row vector before reading a new line from the input file.

        stringstream s(line);   // Create a stringstream object from the line read from the input file.


        /*  Extracts individual comma-separated values from the stringstream-s 
        and store them in the string-word. Comma(,) is used as the delimited.
        Then append each value to the row vector.                          */
        while (getline(s, word, ',')) 
        {
            row.push_back(word);
        }


        // Create a new order object using the values stored in the row vector.
        Order New_Order(&row[0], &row[1], &row[2], &row[3], &row[4]);




        /*                  Check if New_Order is Reject or not                    */
        /*  If New_Order is NOT Reject, the If block will be executed.             */
        /*  Otherise (Reject), the Else block will be executed                     */

        if (New_Order.Check_If_Not_Rejected())      // New_Order is NOT Reject  
        {

             // Find index of the order book for the instrument of the new order.
            int Order_Book_Index = (int)(find(begin(Instrument_List), end(Instrument_List), New_Order.Instrument_Name) - begin(Instrument_List));

            // Retrieve the order book for the instrument of the new order.
            vector<vector<Order>> Order_Book = Order_Book_Library[Order_Book_Index];




            // Side = 1 means buy & Side = 2 means sell.


            if (New_Order.Side == 1)           // Processing buy orders
            {

                /*     Check whether there are any sell orders in order book and if their 
                price is less than or equal to the new buy order price.                */
                while (!(Order_Book[1].empty()) && (Order_Book[1][0].Price <= New_Order.Price)) 
                {
                    // Check if remaining quantity of new buy order matches remaining quantity of first sell order.
                    // If they match exactly, the entire buy order will be executed by the first sell order.
                    if (New_Order.Remaining_Quantity == Order_Book[1][0].Remaining_Quantity) 
                    {
                        New_Order.Status = 2;                       // Set status of new buy order as "Fill"
                        Order_Book[1][0].Status = 2;                // Set status of first sell order as "Fill"
                        New_Order.Execute(F_out);                   // Execute the new buy order and write execution details
                        Order_Book[1][0].Execute(F_out);            // Execute the first sell order and write execution details
                        New_Order.Remaining_Quantity = 0;           // Set remaining quantity of the new buy order to zero
                        Order_Book[1][0].Remaining_Quantity = 0;    // Set remaining quantity of the first sell order to zero
                        Order_Book[1].erase(begin(Order_Book[1]));  // Remove the first sell order from the order book
                        break;
                    } 

                    // Check if remaining quantity of new buy order is greater than remaining quantity of first sell order.
                    else if (New_Order.Remaining_Quantity > Order_Book[1][0].Remaining_Quantity) 
                    {
                        double temp = New_Order.Price;              // Store the original execution price of the new buy order
                        New_Order.Status = 3;                       // Set status of new buy order as "Partial Fill"
                        Order_Book[1][0].Status = 2;                // Set status of first sell order as "Fill"
                        New_Order.Price = Order_Book[1][0].Price;   // Set the execution price to match the sell order price
                        New_Order.Execute(F_out, Order_Book[1][0].Remaining_Quantity);      // Execute the new buy order for the quantity of the first sell order
                        Order_Book[1][0].Execute(F_out);            // Execute the first sell order and write execution details
                        New_Order.Remaining_Quantity = New_Order.Remaining_Quantity - Order_Book[1][0].Remaining_Quantity;      // Update remaining quantity of the new buy order
                        Order_Book[1][0].Remaining_Quantity = 0;    // Set remaining quantity of the first sell order to zero
                        New_Order.Price = temp;                     // Restore the original execution price of the new buy order
                        Order_Book[1].erase(begin(Order_Book[1]));  // Remove the first sell order from the order book
                    } 


                    // Check if remaining quantity of new buy order is less than remaining quantity of first sell order
                    else 
                    {
                        New_Order.Status = 2;                       // Set status of new buy order as "Fill"
                        Order_Book[1][0].Status = 3;                // Set status of first sell order as "Partial Fill"
                        New_Order.Price = Order_Book[1][0].Price;   // Set the execution price to match the sell order price
                        New_Order.Execute(F_out);                   // Execute the new buy order and write execution details
                        Order_Book[1][0].Execute(F_out, New_Order.Remaining_Quantity);      // Execute the first sell order for the quantity of the new buy order
                        Order_Book[1][0].Remaining_Quantity = Order_Book[1][0].Remaining_Quantity - New_Order.Remaining_Quantity;   // Update remaining quantity of the first sell order
                        New_Order.Remaining_Quantity = 0;           // Set remaining quantity of the new buy order to zero
                        break;
                    }
                }

                // If the status of the new buy order is still "New" (not fully matched or partially matched)
                if (New_Order.Status == 0) 
                {
                    New_Order.Execute(F_out);                     
                }

                // If the new buy order has remaining quantity, insert it into the buy-side order book
                if (New_Order.Remaining_Quantity > 0.0) 
                {
                    Insert_Order_into_OB(Order_Book[0], New_Order, 1);  
                }

            }



            else if (New_Order.Side == 2)      // Processing sell orders
            {

                while (!(Order_Book[0].empty()) && (Order_Book[0][0].Price >= New_Order.Price)) 
                {

                    // If remaining quantity of new sell order matches remaining quantity of first buy order        
                    if (New_Order.Remaining_Quantity == Order_Book[0][0].Remaining_Quantity) 
                    {
                        
                        New_Order.Status = 2;                       // Set status of new sell order as "Fill"
                        Order_Book[0][0].Status = 2;                // Set status of first buy order as "Fill"
                        New_Order.Price = Order_Book[0][0].Price;   // Set the execution price to match the buy order price
                        New_Order.Execute(F_out);                   // Execute the new sell order and write execution details
                        Order_Book[0][0].Execute(F_out);            // Execute the first buy order and write execution details
                        New_Order.Remaining_Quantity = 0;           // Set remaining quantity of the new sell order to zero
                        Order_Book[0][0].Remaining_Quantity = 0;    // Set remaining quantity of the first buy order to zero
                        Order_Book[0].erase(begin(Order_Book[0]));  // Remove the first buy order from the order book
                        break;
                    } 

                    // If remaining quantity of new sell order is greater than remaining quantity of the first buy order.
                    else if (New_Order.Remaining_Quantity > Order_Book[0][0].Remaining_Quantity) 
                    {
                        
                        double temp2 = New_Order.Price;             // Store the original execution price of the new sell order
                        New_Order.Status = 3;                       // Set status of new sell order as "Partial Fill"
                        Order_Book[0][0].Status = 2;                // Set status of first buy order as "Fill"
                        New_Order.Price = Order_Book[0][0].Price;   // Set the execution price to match the buy order price
                        New_Order.Execute(F_out, Order_Book[0][0].Remaining_Quantity);      // Execute the new sell order for the quantity of the first buy order
                        New_Order.Remaining_Quantity = New_Order.Remaining_Quantity - Order_Book[0][0].Remaining_Quantity;      // Update remaining quantity of the new sell order
                        Order_Book[0][0].Execute(F_out);            // Execute the first buy order and write execution details
                        Order_Book[0][0].Remaining_Quantity = 0;    // Set remaining quantity of the first buy order to zero
                        New_Order.Price = temp2;                    // Restore the original execution price of the new sell order
                        Order_Book[0].erase(begin(Order_Book[0]));  // Remove the first buy order from the order book
                    } 

                    // If remaining quantity of new sell order is less than remaining quantity of first buy order.
                    else 
                    {
                        New_Order.Status = 2;                       // Set status of new sell order as "Fill"
                        Order_Book[0][0].Status = 3;                // Set status of first buy order as "Partial Fill"
                        New_Order.Price = Order_Book[0][0].Price;   // Set the execution price to match the buy order price
                        New_Order.Execute(F_out);                   // Execute the new sell order and write execution details
                        Order_Book[0][0].Execute(F_out, New_Order.Remaining_Quantity);      // Execute the first buy order for the quantity of the new sell order
                        Order_Book[0][0].Remaining_Quantity = Order_Book[0][0].Remaining_Quantity - New_Order.Remaining_Quantity;  // Update remaining quantity of the first buy order
                        New_Order.Remaining_Quantity = 0;           // Set remaining quantity of the new sell order to zero
                        break;
                    }

                }


                if (New_Order.Status == 0) 
                {
                    // If the new sell order was not fully matched or partially matched
                    New_Order.Execute(F_out);
                }

                if (New_Order.Remaining_Quantity > 0.0) 
                {
                    // If there is remaining quantity in the new sell order after matching
                    Insert_Order_into_OB(Order_Book[1], New_Order, 2);  // Insert the new sell order into the order book
                }

            }


            // Update the order book in the order book library.
            Order_Book_Library[Order_Book_Index] = Order_Book; 


        } 


        else   // New_Order is Reject                           
        {
            New_Order.Execute(F_out);
        }




    }


    F_in.close();           // Close the input file stream.
    F_out.close();          // Close the output file stream.


}




int main(void) 
{

    /********* PUT THE INPUT FILE NAME BELOW *********/
    string Input_File_Name = "TestOrders.csv";  

    Process_Orders_and_Update_Books(Input_File_Name);

    return 0;

}




/*------------------------------------------- END -------------------------------------------*/