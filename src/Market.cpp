// Copyright (c) 2017 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "Market.h"

#include <functional> 
#include <cctype>
#include <locale>

#include "RessourcesManager.h"

namespace {
    ///////////////////////
    // depth display helper
    void displayDepthLevel(std::ostream & out, const liquibook::book::DepthLevel & level)
    {
        out << "\tPrice "  <<  level.price();
        out << " Count: " << level.order_count();
        out << " Quantity: " << level.aggregate_qty();
        if(level.is_excess())
        {
            out << " EXCESS";
        }
        out << " Change id#: " << level.last_change();
        out << std::endl;
    }

    void publishDepth(std::ostream & out, const orderentry::BookDepth & depth)
    {
        liquibook::book::ChangeId published = depth.last_published_change();
        bool needTitle = true;
        // Iterate awkwardly
        auto pos = depth.bids();
        auto back = depth.last_bid_level();
        bool more = true;
        while(more)
        {
            if(pos->aggregate_qty() !=0 && pos->last_change() > published)
            {
                if(needTitle)
                {
                    out << "\n\tBIDS:\n";
                    needTitle = false;
                }
                displayDepthLevel(out, *pos);
            }
            ++pos;
            more = pos != back;
        }

        needTitle = true;
        pos = depth.asks();
        back = depth.last_ask_level();
        more = true;
        while(more)
        {
            if(pos->aggregate_qty() !=0 && pos->last_change() > published)
            {
                if(needTitle)
                {
                    out << "\n\tASKS:\n";
                    needTitle = false;
                }
                displayDepthLevel(out, *pos);
            }
            ++pos;
            more = pos != back;
        }
    }
}

namespace orderentry
{

uint32_t Market::orderIdSeed_ = 0;

Market::Market(std::shared_ptr<RessourcesManager> ressourcesManager,std::ostream * out)
: logFile_(out),m_ressourcesManager(ressourcesManager)
{
}

Market::~Market()
{
}

/////////////////////////////////////
// Implement OrderListener interface

void 
Market::on_accept(const OrderPtr& order)
{
    order->onAccepted();
    out() << "\tAccepted: " <<*order<< std::endl;
    m_ressourcesManager->on_accept(order);
}

void 
Market::on_reject(const OrderPtr& order, const char* reason)
{
    order->onRejected(reason);
    out() << "\tRejected: " <<*order<< ' ' << reason << std::endl;
    m_ressourcesManager->on_reject(order,reason);
}

void 
Market::on_fill(const OrderPtr& order, 
    const OrderPtr& matched_order, 
    liquibook::book::Quantity fill_qty, 
    liquibook::book::Cost fill_cost)
{
    order->onFilled(fill_qty, fill_cost);
    matched_order->onFilled(fill_qty, fill_cost);
    out() << (order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' <<*order<< std::endl;
    out() << (matched_order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' << *matched_order << std::endl;
    m_ressourcesManager->on_fill(order,matched_order,fill_qty,fill_cost);
}

void 
Market::on_cancel(const OrderPtr& order)
{
    order->onCancelled();
    out() << "\tCanceled: " << *order<< std::endl;
    m_ressourcesManager->on_cancel(order);
}

void Market::on_cancel_reject(const OrderPtr& order, const char* reason)
{
    order->onCancelRejected(reason);
    out() << "\tCancel Reject: " <<*order<< ' ' << reason << std::endl;
    m_ressourcesManager->on_cancel_reject(order,reason);
}

void Market::on_replace(const OrderPtr& order, 
    const int32_t& size_delta, 
    liquibook::book::Price new_price)
{
    order->onReplaced(size_delta, new_price);
    out() << "\tModify " ;
    if(size_delta != liquibook::book::SIZE_UNCHANGED)
    {
        out() << " QUANTITY  += " << size_delta;
    }
    if(new_price != liquibook::book::PRICE_UNCHANGED)
    {
        out() << " PRICE " << new_price;
    }
    out() <<*order<< std::endl;
    m_ressourcesManager->on_replace(order,size_delta,new_price);
}

void 
Market::on_replace_reject(const OrderPtr& order, const char* reason)
{
    order->onReplaceRejected(reason);
    out() << "\tReplace Reject: " <<*order<< ' ' << reason << std::endl;
    m_ressourcesManager->on_replace_reject(order,reason);
}

////////////////////////////////////
// Implement TradeListener interface

void 
Market::on_trade(const OrderBook* book, 
    liquibook::book::Quantity qty, 
    liquibook::book::Cost cost)
{
    out() << "\tTrade: " << qty <<  ' ' << book->symbol() << " Cost "  << cost  << std::endl;
}

/////////////////////////////////////////
// Implement OrderBookListener interface

void 
Market::on_order_book_change(const OrderBook* book)
{
    out() << "\tBook Change: " << ' ' << book->symbol() << std::endl;
}



/////////////////////////////////////////
// Implement BboListener interface
void 
Market::on_bbo_change(const DepthOrderBook * book, const BookDepth * depth)
{
    out() << "\tBBO Change: " << ' ' << book->symbol() 
        << (depth->changed() ? " Changed" : " Unchanged")
        << " Change Id: " << depth->last_change()
        << " Published: " << depth->last_published_change()
        << std::endl;

}

/////////////////////////////////////////
// Implement DepthListener interface
void 
Market::on_depth_change(const DepthOrderBook * book, const BookDepth * depth)
{
    out() << "\tDepth Change: " << ' ' << book->symbol();
    out() << (depth->changed() ? " Changed" : " Unchanged")
        << " Change Id: " << depth->last_change()
        << " Published: " << depth->last_published_change();
    publishDepth(out(), *depth);
    out() << std::endl;
}


OrderBookPtr
Market::addBook(const std::string & symbol)
{
    OrderBookPtr result;
    out() << "Create new depth order book for " << symbol << std::endl;
    DepthOrderBookPtr depthBook = std::make_shared<DepthOrderBook>(symbol);
    depthBook->set_bbo_listener(this);
    depthBook->set_depth_listener(this);
    result = depthBook;
    result->set_order_listener(this);
    result->set_trade_listener(this);
    result->set_order_book_listener(this);
    books_[symbol] = result;
    return result;
}

bool
Market::symbolIsDefined(const std::string & symbol)
{
    auto book = books_.find(symbol);
    return book != books_.end();
}

void Market::setOrderSeed(uint32_t orderIdSeed)
{
    orderIdSeed_=orderIdSeed;
}

int Market::addOrder(int PCId,std::string side,std::string symbol,int quantity,int price,int fee,int stopPrice,bool aon,bool ioc)
{
    std::string orderId = std::to_string(++orderIdSeed_);
    m_ressourcesManager->UpdateOrderSeed(orderIdSeed_);
    return addOrder(orderId,PCId,side,symbol,quantity,price,fee,stopPrice,aon,ioc);
}

int Market::addOrder(std::string orderId,int PCId,std::string side,std::string symbol,int quantity,int price,int fee,int stopPrice,bool aon,bool ioc)
{
    int iorderId=std::stoi(orderId);
    OrderPtr order = std::make_shared<Order>(orderId,PCId, side == "BUY", quantity, symbol, price, stopPrice, aon, ioc,fee);
    const liquibook::book::OrderConditions AON(liquibook::book::oc_all_or_none);
    const liquibook::book::OrderConditions IOC(liquibook::book::oc_immediate_or_cancel);
    const liquibook::book::OrderConditions NOC(liquibook::book::oc_no_conditions);

    const liquibook::book::OrderConditions conditions = 
        (aon ? AON : NOC) | (ioc ? IOC : NOC);


    auto book = findBook(symbol);
    if(!book)
    {
        throw std::runtime_error(std::string("--No order book for symbol")+symbol);
        return -1;
    }

    order->onSubmitted();
    out() << "ADDING order:  " << *order << std::endl;

    orders_[orderId] = order;
    book->add(order, conditions);
    return iorderId;
}

OrderBookPtr
Market::findBook(const std::string & symbol)
{
    OrderBookPtr result;
    auto entry = books_.find(symbol);
    if(entry != books_.end())
    {
        result = entry->second;
    }
    return result;
}

void Market::ListPriceBook(std::function<void(std::string)> func,std::string GoodsName)
{
    if(symbolIsDefined(GoodsName))
    {
        auto book =findBook(GoodsName);
        if(!book)
        {
            throw std::runtime_error(std::string("No book with name ")+GoodsName);
        }
        else
        {
            auto depthInstance=static_cast<DepthOrderBook*>(book.get())->depth();
            std::string message("Buy:\n");
            for(auto depthI=depthInstance.last_bid_level();depthI>=depthInstance.bids();depthI--)
            {
                if(depthI->aggregate_qty()>0)
                {
                    message+=std::to_string(depthI->aggregate_qty())+std::string(" @ ")+std::to_string(depthI->price())+std::string("\n");
                }
            }
            message+=std::string("Sell:\n");
            std::for_each(depthInstance.asks(),depthInstance.last_ask_level(),[&message](auto depthI)
            {
                if(depthI.aggregate_qty()>0)
                {
                    message+=std::to_string(depthI.aggregate_qty())+std::string(" @ ")+std::to_string(depthI.price())+std::string("\n");
                }
            });
            func(message);
        }
    }
}

void Market::ListAllMarketPrices(std::function<void(std::string)> func)
{
    std::string message;
    for_each(books_.begin(),books_.end(),[&message](auto elem)
    {
        auto book =elem.second;
        if(!book)
        {
            throw std::runtime_error(std::string("No book with name ")+elem.first);
        }
        else
        {
            auto depthInstance=static_cast<DepthOrderBook*>(book.get())->depth();
            std::string buy_agregate;
            for(auto depthI=depthInstance.last_bid_level();depthI>=depthInstance.bids();depthI--)
            {
                if(depthI->aggregate_qty()>0)
                {
                    buy_agregate+=std::to_string(depthI->aggregate_qty())+std::string(" @ ")+std::to_string(depthI->price())+std::string("\n");
                }
            }
            std::string trade_message;
            if(!buy_agregate.empty())
            {
                trade_message=ColorRed(std::string("Buy"))+std::string(":\n")+buy_agregate;
            }
            std::string sell_agregate;
            std::for_each(depthInstance.asks(),depthInstance.last_ask_level(),[&sell_agregate](auto depthI)
            {
                if(depthI.aggregate_qty()>0)
                {
                    sell_agregate+=std::to_string(depthI.aggregate_qty())+std::string(" @ ")+std::to_string(depthI.price())+std::string("\n");
                }
            });
            if(!sell_agregate.empty())
            {
                trade_message=ColorRed(std::string("Sell"))+std::string(":\n")+sell_agregate;
            }
            if(!trade_message.empty())
            {
                message+=ColorRed(elem.first)+std::string(":\n")+trade_message;
            }
        }
    });
    func(message);
}

const OrderPtr Market::GetOrder(std::string orderId)
{
    try
    {
        return orders_.at(orderId);
    }
    catch(std::out_of_range& ex)
    {
        throw std::out_of_range("OrderId doesn't exists");
    }
}


}  // namespace orderentry

