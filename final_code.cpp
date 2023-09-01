
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

class Order_book
{

public:
    string client_id;
    string ord_id;
    string inst;
    int side;
    double price;
    int quantity;
    int status_int;
    string status;
    string price_string;
    string reason = "-"; 

    Order_book(string client_id, string inst, int side, int quantity, double price, string price_string)
        : client_id(client_id), inst(inst), side(side), quantity(quantity), price(price), price_string(price_string) {}
};

bool compareBuy(const Order_book &order1, const Order_book &order2)
{
    if (order1.price == order2.price)
        return false; // Preserve original order for equal prices
    return order1.price > order2.price;
}

bool compareSell(const Order_book &order1, const Order_book &order2)
{
    if (order1.price == order2.price)
        return false; // Preserve original order for equal prices
    return order1.price < order2.price;
}

string getFormattedTime()
{
   
    auto currentTime = chrono::system_clock::now();                      // Get the current time
    time_t currentTimeT = chrono::system_clock::to_time_t(currentTime); // Convert current time to a time_t for formatting
    tm *timeInfo = localtime(&currentTimeT);                             // Format the time as "YYYYMMDD-HHMMSS.sss"
    char formattedTime[20];
    strftime(formattedTime, sizeof(formattedTime), "%Y%m%d-%H%M%S", timeInfo);
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(currentTime.time_since_epoch()) % 1000; // Get milliseconds
    stringstream ss;
    ss << formattedTime << "." << setfill('0') << setw(3) << milliseconds.count();
    return ss.str();
}

void ProcessOrders(vector<Order_book> &buy, vector<Order_book> &sell, int side, int count, const string &client_id, const string &instrument, int quantity, const string &price_string, double price, ofstream &output, Order_book &order)
{

    int s_check = 0;
    int b_check = 0;

    if (side == 1)
    { // Buy

        if (sell.empty())
        {
            order.status_int = 0;
            order.status = "New";

            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << ",  -  ," << getFormattedTime() << endl;

            buy.push_back(order);
            sort(buy.begin(), buy.end(), compareBuy);
        }

        else
        {                               // sell vector not empty
            if (sell[0].price <= price) // while(!sell.empty() && sell[0].price <= price)
            {
                
                while ((!sell.empty()) && (sell[0].price <= price))
                {

                    if (sell[0].quantity == quantity)
                    {
                        order.status_int = 2;
                        order.status = "Fill";
                        sell[0].status_int = 2;
                        sell[0].status = "Fill";
                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << sell[0].ord_id << "," << sell[0].client_id << "," << sell[0].inst << "," << sell[0].side << "," << sell[0].status << "," << quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        sell.erase(sell.begin());

                       
                        break;
                    }

                    else if (sell[0].quantity > quantity)
                    {
                        order.status_int = 2;
                        order.status = "Fill";
                        sell[0].status_int = 3;
                        sell[0].status = "Pfill";
                        sell[0].quantity = sell[0].quantity - quantity;
                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << sell[0].ord_id << "," << sell[0].client_id << "," << sell[0].inst << "," << sell[0].side << "," << sell[0].status << "," << quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        
                        break;
                    }

                    else
                    { // sell[0].quantity < quantity
                        order.status_int = 3;
                        order.status = "Pfill";
                        sell[0].status_int = 2;
                        sell[0].status = "Fill";
                        order.quantity = order.quantity - sell[0].quantity;
                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << sell[0].quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << sell[0].ord_id << "," << sell[0].client_id << "," << sell[0].inst << "," << sell[0].side << "," << sell[0].status << "," << sell[0].quantity << "," << sell[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        if (b_check == 0)   //Check if its already inside vector
                        {

                            sell.erase(sell.begin());
                            buy.push_back(order);
                            sort(buy.begin(), buy.end(), compareBuy);
                            b_check = 1; // To make sure this order is already inside vector
                        }

                        else    // The order is already inside vector
                        {
                            sell.erase(sell.begin());
                            buy[0].quantity = buy[0].quantity - sell[0].quantity;
                        }
                    }
                }
            }

            else
            { // sell[0].price is higher than buying price. So just insert the object to buy vector
                
                order.status_int = 0;
                order.status = "New";

                output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << ",  -  ," << getFormattedTime() << endl;

                buy.push_back(order);
                sort(buy.begin(), buy.end(), compareBuy);
            }
        }
    }

    else if (side == 2)
    { // Sell
        if (buy.empty())
        {
            order.status_int = 0;
            order.status = "New";

            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << ",  -  ," << getFormattedTime() << endl;

            sell.push_back(order);
            sort(sell.begin(), sell.end(), compareSell);
            
        }

        else
        { // buy vector not empty
            if (buy[0].price >= price)
            {

                while ((!buy.empty()) && (buy[0].price >= price))
                {

                    if (buy[0].quantity == quantity)
                    {
                        order.status_int = 2;
                        order.status = "Fill";
                        buy[0].status_int = 2;
                        buy[0].status = "Fill";
                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << buy[0].ord_id << "," << buy[0].client_id << "," << buy[0].inst << "," << buy[0].side << "," << buy[0].status << "," << quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        buy.erase(buy.begin());

                        
                        break;
                    }

                    else if (buy[0].quantity > quantity)
                    {

                        order.status_int = 2;
                        order.status = "Fill";
                        buy[0].status_int = 3;
                        buy[0].status = "Pfill";
                        buy[0].quantity = buy[0].quantity - quantity;

                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << buy[0].ord_id << "," << buy[0].client_id << "," << buy[0].inst << "," << buy[0].side << "," << buy[0].status << "," << quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        
                        break;
                    }

                    else
                    { // buy[0].quantity< quantity
                        order.status_int = 3;
                        order.status = "Pfill";
                        buy[0].status_int = 2;
                        buy[0].status = "Fill";

                        order.quantity = order.quantity - buy[0].quantity;
                        output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << buy[0].quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;
                        output << "ord" << buy[0].ord_id << "," << buy[0].client_id << "," << buy[0].inst << "," << buy[0].side << "," << buy[0].status << "," << buy[0].quantity << "," << buy[0].price_string << ",  -  ," << getFormattedTime() << endl;

                        if (s_check == 0)
                        {

                            buy.erase(buy.begin());
                            sell.push_back(order);
                            sort(sell.begin(), sell.end(), compareSell);
                            s_check = 1; // To make sure this order is already inside vector
                        }

                        else
                        { // The order is already inside vector.
                            buy.erase(buy.begin());
                            sell[0].quantity = sell[0].quantity - buy[0].quantity;
                        }
                    }
                }
            }

            else
            { // price is higher than buy[0].price. So just insert the object to buy vector
                order.status = "New";
                order.status_int = 0;
                output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << ",  -  ," << getFormattedTime() << endl;
                sell.push_back(order);
                sort(sell.begin(), sell.end(), compareSell);
            }
        }
    }
}

int main()
{
    

    vector<Order_book> buy;
    vector<Order_book> sell;

    vector<Order_book> buyRose;
    vector<Order_book> sellRose;

    vector<Order_book> buyLavender;
    vector<Order_book> sellLavender;

    vector<Order_book> buyLotus;
    vector<Order_book> sellLotus;

    vector<Order_book> buyTulip;
    vector<Order_book> sellTulip;

    vector<Order_book> buyOrchid;
    vector<Order_book> sellOrchid;

    ifstream file;
    file.open("order.csv"); // order.csv file has to be in the same folder as the code

    if (!file.is_open())
    {
        cout << "Failed to open the file." << endl;
        return 1;
    }

    ofstream output;
    output.open("execution_rep.csv");

    if (!output.is_open())
    {
        cout << "Error creating file." << endl;
        return 1;
    }

    output << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Transaction Time" << endl;

    string header;
    getline(file, header);

    int count = 1;

    // Process multiple data lines
    string dataLine;
    while (getline(file, dataLine))
    {
        // Split the data line into individual values
        stringstream ss(dataLine);
        string value;
        vector<string> values; // Store values in a vector
        while (getline(ss, value, ','))
        {
            values.push_back(value);
        }

        
        string client_id = values[0];
        string instrument = values[1];
        // int side = stoi(values[2]);
        // int quantity = stoi(values[3]);
        int side, quantity;

        try
        {
            side = stoi(values[2]);
        }
        catch (const std::invalid_argument &)
        {
            output << "ord" << count << "," << client_id << "," << instrument << "," << values[2] << ","
                   << "Reject"
                   << "," << values[3] << "," << values[4] << "," <<"Invalid side,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        try
        {
            quantity = stoi(values[3]);
        }
        catch (const std::invalid_argument &)
        {
            output << "ord" << count << "," << client_id << "," << instrument << "," << values[2] << ","
                   << "Reject"
                   << "," << values[3] << "," << values[4] << "," <<"Invalid size,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        string price_string = values[4];
        // double price = stod(values[4]);
        double price;

        try
        {
            price = stod(values[4]);
        }
        catch (const std::invalid_argument &)
        {
            output << "ord" << count << "," << client_id << "," << instrument << "," << values[2] << ","
                   << "Reject"
                   << "," << values[3] << "," << values[4] << "," <<"Invalid price,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        Order_book order(client_id, instrument, side, quantity, price, price_string);
        order.ord_id = to_string(count);

        if (order.client_id.empty())
        {
            order.status = "Reject";
            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << "," <<"Invalid Client ID,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        else if (order.inst.empty() || (instrument != "Rose" && instrument != "Lavender" && instrument != "Lotus" && instrument != "Tulip" && instrument != "Orchid"))
        {
            order.status = "Reject";
            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << "," <<"Invalid instrument,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        else if (side != 1 && side != 2)
        {
            order.status = "Reject";
            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << "," <<"Invalid side,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        else if (price <= 0)
        {
            order.status = "Reject";
            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << "," <<"Invalid price,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        else if (quantity < 10 || quantity > 1000 || quantity % 10 != 0)
        {
            order.status = "Reject";
            output << "ord" << count << "," << client_id << "," << instrument << "," << side << "," << order.status << "," << quantity << "," << price_string << "," <<"Invalid size,"<< getFormattedTime() << endl;
            count++;
            continue;
        }

        

        if (instrument == "Rose")
            ProcessOrders(buyRose, sellRose, side, count, client_id, instrument, quantity, price_string, price, output, order);
        else if (instrument == "Lavender")
            ProcessOrders(buyLavender, sellLavender, side, count, client_id, instrument, quantity, price_string, price, output, order);
        else if (instrument == "Lotus")
            ProcessOrders(buyLotus, sellLotus, side, count, client_id, instrument, quantity, price_string, price, output, order);
        else if (instrument == "Tulip")
            ProcessOrders(buyTulip, sellTulip, side, count, client_id, instrument, quantity, price_string, price, output, order);
        else if (instrument == "Orchid")
            ProcessOrders(buyOrchid, sellOrchid, side, count, client_id, instrument, quantity, price_string, price, output, order);
        count++;
    }

    file.close();
    output.close();

    return 0;
}
