#include "portfolio.h"

#include <QVariant>
#include <QJsonArray>

void LoadUp::save(QJsonObject &json) const {
  json.insert("time_stamp", QJsonValue::fromVariant(QVariant(time_stamp)));
  json.insert("quantity", QJsonValue(quantity));
}

void LoadUp::load(const QJsonObject &json) {
  if (json.contains("time_stamp") && json["time_stamp"].isDouble()) {
    time_stamp = json["time_stamp"].toVariant().toLongLong();
  }

  if (json.contains("quantity") && json["quantity"].isDouble()) {
    quantity = json["quantity"].toDouble();
  }
}

// Returns how much quantity is recorded.
qreal StockRecord::quantityRecorded() const {
  qreal quantity = 0;

  for (auto &x : record) {
    quantity += x.second;   // x = {price, quantity}
  }

  return quantity;
}

// Returns the current market value, based on the API.
qreal StockRecord::marketValuePerQuantity() const {
  return stock->getLatestClosedPrice();
}

// Returns the cost of buying owned quantity.
qreal StockRecord::costBasis() const {
  qreal base_cost = 0;

  for (auto &x : record) {
    base_cost += x.first * x.second;
  }

  return base_cost;
}

// Returns the value of owned quantity.
qreal StockRecord::valuation() const {
  return marketValuePerQuantity() * quantityRecorded();
}

// Returns the gain/loss of owned quantity. DOES NOT TAKE ACCOUNT OF SOLD
// QUANTITY.
qreal StockRecord::totalGainLoss() const {
  return valuation() - costBasis();
}

// Adds a new pair of price-quantity to the record.
void StockRecord::addStock(qreal price, qreal quantity) {
  record.insert(QPair<qreal, qreal>(price, quantity));
}

// Remove a part of quantity from the record. Since we can remove any arbitrary
// own quantity, it will make more senses to remove quantity with lowest base
// costs / buying prices.
void StockRecord::removeStock(qreal quantity) {
  if (quantity > quantityRecorded()) {
    return;
  }

  while (quantity > 1e-9) { // basically "while (quantity > 0)"
    Q_ASSERT(!record.empty());

    auto x = *record.begin();
    record.erase(record.begin());

    auto minuend = qMin(quantity, x.second);
    quantity -= minuend;
    x.second -= minuend;

    if (abs(x.second) > 1e-9) {
      record.insert(x);
      // loop should end after this
    }
  }

  return;
}

// Returns the current market value of the owned stocks.
qreal Portfolio::stockValuation() const {
  qreal res = 0;

  foreach (StockRecord s, stock_records) {
    res += s.valuation();
  }

  return res;
}

// Returns the total value of the portfolio. This includes current market value
// of owned stocks + raw money
qreal Portfolio::valuation() const {
  return stockValuation() + current_money;
}

void Portfolio::addStockToWatchList(QString &symbol) {
  if (stock_watch_list.indexOf(symbol) >= 0) {
    return;
  }

  stock_watch_list.append(symbol);
}

void Portfolio::removeStockFromWatchList(QString &symbol) {
  int index = stock_watch_list.indexOf(symbol);

  if (index >= 0) {
    stock_watch_list.removeAt(index);
  }
}

// Returns a pointer to the stock, if it appears in watchlistStocks
Stock *Portfolio::getStock(QString symbol) {
  for (auto stockptr : watchlistStocks) {
    if (stockptr->getSymbol() == helper::toStdString(symbol)) {
      return stockptr;
    }
  }

  return nullptr;
}

// Adds a trading order to the portfolio, if and only if it satisfies the
// "no-negative-numbers" constrains after ordering it.
void Portfolio::addTradingOrder(TradingOrder *trading_order) {
  QString symbol = trading_order->getSymbol();

  // init if not exist
  if (!stock_records.contains(symbol)) {
    stock_records[symbol] = StockRecord(getStock(symbol));
  }

  qreal trade_price = trading_order->getValuePerQuantity();
  qreal trade_quantity = trading_order->getQuantity();
  qreal quantity_recorded = stock_records[symbol].quantityRecorded();

  if (trading_order->getAction() == TradingOrder::TradingAction::Buy) {
    if (trade_price * trade_quantity > current_money) {   // can't buy
      return;
    } else {
      current_money -= trade_price * trade_quantity;
      stock_records[symbol].addStock(trade_price, trading_order->getQuantity());
      trading_order_history.push_back(trading_order);
    }
  } else if (trading_order->getAction() == TradingOrder::TradingAction::Sell) {
    if (quantity_recorded < trade_quantity) {   // can't sell
      return;
    } else {
      current_money += trade_price * trade_quantity;
      stock_records[symbol].removeStock(trading_order->getQuantity());
      trading_order_history.push_back(trading_order);
    }
  } else {  // do not need to implement this case (SellShort)
    return;
  }
}

// This function has no uses for now.
void Portfolio::addLoadUp(LoadUp *load_up) {
  this->current_money += load_up->getQuantity();
  load_up_history.push_back(load_up);
}

// Computes stock_records based on trading_order_history. There is a few reasons
// for its existence:
//  - Old save files do not have stock_records before, so constructing from
//    trading_order_history is necessary
//  - To ease the implementation of other functions (call this before doing
//    anything will ensure stock_records will have the correct value.
//
// See also: addTradingOrder
void Portfolio::computeRecordFromHistory() {
  stock_records = QHash<QString, StockRecord>();
  current_money = initial_money;

  for (auto &trading_order : trading_order_history) {
    QString symbol = trading_order->getSymbol();

    // init if not exist
    if (!stock_records.contains(symbol)) {
      stock_records[symbol] = StockRecord(getStock(symbol));
    }

    qreal trade_price = trading_order->getValuePerQuantity();
    qreal trade_quantity = trading_order->getQuantity();
    qreal quantity_recorded = stock_records[symbol].quantityRecorded();

    if (trading_order->getAction() == TradingOrder::TradingAction::Buy) {
      if (trade_price * trade_quantity > current_money) {   // can't buy
        continue;
      } else {
        current_money -= trade_price * trade_quantity;
        stock_records[symbol].addStock(trade_price, trading_order->getQuantity());
      }
    } else if (trading_order->getAction() == TradingOrder::TradingAction::Sell) {
      if (quantity_recorded < trade_quantity) {   // can't sell
        continue;
      } else {
        current_money += trade_price * trade_quantity;
        stock_records[symbol].removeStock(trading_order->getQuantity());
      }
    } else {  // do not need to implement this case (SellShort)
      continue;
    }
  }
}

// Returns current owned stocks - stocks that appear in stock_records and have
// positive quantity.
QVector<QString> Portfolio::currentOwnedStock() const {
  QHash<QString, StockRecord>::const_iterator it = stock_records.constBegin();

  QSet<QString> owned;

  while (it != stock_records.constEnd()) {
    if (it.value().quantityRecorded() > 1e-9) { // the same as > 0
      owned.insert(it.key());
    }

    ++it;
  }

  QVector <QString> owned_qvector;

  foreach (QString v, owned) {
    owned_qvector.push_back(v);
  }

  return owned_qvector;
}

// Return owned quantity of a stock.
qreal Portfolio::getOwnedQuantity(QString symbol) const {
  if (!stock_records.contains(symbol)) {
    return 0;
  }

  return stock_records.value(symbol).quantityRecorded();
}

qreal Portfolio::getOwnedQuantity(std::string symbol) const {
  return getOwnedQuantity(helper::toQString(symbol));
}

// Return the value of owned quantity of a stock.
qreal Portfolio::getMarketValue(QString symbol) const {
  if (!stock_records.contains(symbol)) {
    return 0;
  }

  return stock_records.value(symbol).valuation();
}

qreal Portfolio::getMarketValue(std::string symbol) const {
  return getMarketValue(helper::toQString(symbol));
}

// Return the cost basis / buying prices of owned quantity of a stock.
qreal Portfolio::getCostBasis(QString symbol) const {
  if (!stock_records.contains(symbol)) {
    return 0;
  }

  return stock_records.value(symbol).costBasis();
}

qreal Portfolio::getCostBasis(std::string symbol) const {
  return getCostBasis(helper::toQString(symbol));
}

// Return the difference between current value and cost basis of owned quantity
// of a stock.
qreal Portfolio::getTotalGainLoss(QString symbol) const {
  if (!stock_records.contains(symbol)) {
    return 0;
  }

  return stock_records.value(symbol).totalGainLoss();
}

qreal Portfolio::getTotalGainLoss(std::string symbol) const {
  return getTotalGainLoss(helper::toQString(symbol));
}

// Return how much the market value of owned quantity of a stock is weighted in
// the portfoilio.
// The formula: market_value / portfoilo.valuation * 100
qreal Portfolio::getPercentOfAccount(QString symbol) const {
  if (!stock_records.contains(symbol)) {
    return 0;
  }

  return stock_records.value(symbol).valuation() / valuation() * 100;
}

qreal Portfolio::getPercentOfAccount(std::string symbol) const {
  return getPercentOfAccount(helper::toQString(symbol));
}

void Portfolio::save(QJsonObject &json) const {
  json.insert("id", QJsonValue(id));
  json.insert("initial_money", QJsonValue(initial_money));
  json.insert("current_money", QJsonValue(current_money));

  QJsonArray watchlist = QJsonArray::fromStringList(stock_watch_list);
  json.insert("stock_watch_list", watchlist);

  QJsonArray trading_orders;

  for (int i = 0; i < trading_order_history.size(); ++i) {
    QJsonObject order;
    trading_order_history.at(i)->write(order);
    trading_orders.append(order);
  }

  json.insert("trading_order_history", trading_orders);

  QJsonArray load_ups;

  for (int i = 0; i < load_up_history.size(); ++i) {
    QJsonObject load_up;
    load_up_history.at(i)->save(load_up);
    load_ups.append(load_up);
  }

  json.insert("load_up_history", load_ups);
}

void Portfolio::load(const QJsonObject &json) {
  if (json.contains("id") && json["id"].isString()) {
    id = json["id"].toString();
  }

  if (json.contains("initial_money") && json["initial_money"].isDouble()) {
    initial_money = json["initial_money"].toDouble();
  }

  if (json.contains("current_money") && json["current_money"].isDouble()) {
    current_money = json["current_money"].toDouble();
  }

  if (json.contains("stock_watch_list") && json["stock_watch_list"].isArray()) {
    QJsonArray symbols = json["stock_watch_list"].toArray();

    for (int i = 0; i < symbols.size(); ++i) {
      if (symbols.at(i).isString()) {
        stock_watch_list.append(symbols.at(i).toString());
      }
    }
  }

  if (json.contains("trading_order_history") &&
      json["trading_order_history"].isArray()) {
    QJsonArray trading_orders = json["trading_order_history"].toArray();

    for (int i = 0; i < trading_orders.size(); ++i) {
      if (trading_orders.at(i).isObject()) {
        TradingOrder *order = new TradingOrder();
        order->read(trading_orders.at(i).toObject());
        trading_order_history.push_back(order);
      }
    }
  }

  if (json.contains("load_up_history") && json["load_up_history"].isArray()) {
    QJsonArray load_ups = json["load_up_history"].toArray();

    for (int i = 0; i < load_ups.size(); ++i) {
      if (load_ups.at(i).isObject()) {
        LoadUp *load_up = new LoadUp();
        load_up->load(load_ups.at(i).toObject());
        load_up_history.push_back(load_up);
      }
    }
  }
}

Portfolio::~Portfolio() {
  for (int i = 0; i < trading_order_history.size(); ++i) {
    delete trading_order_history.value(i);
  }

  for (int i = 0; i < load_up_history.size(); ++i) {
    delete load_up_history.value(i);
  }
}
