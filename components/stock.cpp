#include "stock.hpp"

Stock::Stock(std::string stock_name, std::string symbol){
    this->stock_name = stock_name;
    this->symbol = symbol;
}
std::string Stock::get_name(){
    return stock_name;
}
std::string Stock::get_symbol(){
    return symbol;
}

StockByInterval::StockByInterval(std::string stock_name, std::string symbol): Stock(stock_name, symbol){
    current_interval = "5"; //by default we get 5 minutes data
}
StockByInterval::StockByInterval(std::string stock_name, std::string symbol, std::string interval): Stock(stock_name, symbol){
    if ((interval  == "1")||(interval == "D")){
        current_interval = interval;
    }
    else{
        current_interval = "5"; //incorrectly specified interval results in a 5minutes interval by default.
    }
}
std::string StockByInterval::get_current_interval(){
    return current_interval;
}
std::vector<std::map<std::string, double>> StockByInterval::get_stock_data(){
    return stock_data;
}
void StockByInterval::update_interval(std::string interval){
    if ((interval  == "1")||(interval == "D")){
        current_interval = interval;
    }
    else{
        current_interval = "5"; //incorrectly specified interval results in a 5minutes interval by default.
    }

}
//to be done
void StockByInterval::data_update(){

}

// #include <iostream>
// int main()
// {
//     std::string stock_name, symbol, interval;
//     std::cout << "please write the name of your stock:" << std::endl;
//     std::cin >> stock_name;
//     std::cout << "please write the symbol of your stock:" << std::endl;
//     std::cin >> symbol;
//     std::cout << "please write the interval after which you want your stock to be updated. Select values in ['1', '5','D']" << std::endl;
//     std::cin >> interval;
//     StockByInterval stock1 = StockByInterval(stock_name, symbol, interval);
//     std::cout << stock1.get_name() << std::endl;
//     std::cout << stock1.get_symbol() << std::endl;
//     std::cout << stock1.get_current_interval() << std::endl;
//     stock1.update_interval("D");
//     std::cout << stock1.get_current_interval() << std::endl;
//     stock1.update_interval("minute");
//     std::cout << stock1.get_current_interval() << std::endl;
//     return 0;
// }



