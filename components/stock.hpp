#ifndef STOCK_HPP
#define STOCK_HPP

#include <string>
#include <vector>
#include <map>


class Stock {
protected:
    std::string stock_name;
    std::string symbol; // represents stock_name
public:
    Stock(std::string, std::string);
    std::string get_name();
    std::string get_symbol();
};

class StockByInterval: public Stock{
private:
    std::string current_interval;  // variable that indicates the current interval. Takes values in ["1", "5", "D"] 1minute,5minutes and 1day respectively
    std::vector<std::map<std::string, double>> stock_data;
public:
    StockByInterval(std::string symbol, std::string stock_name);
    StockByInterval(std::string symbol, std::string stock_name, std::string);
    std::string get_current_interval();
    std::vector<std::map<std::string, double>> get_stock_data();
    void update_interval(std::string);
    void data_update();
};
#endif // STOCK_HPP
