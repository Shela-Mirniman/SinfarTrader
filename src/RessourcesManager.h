#pragma once
#include "Database.h"
#include <iostream>
#include <memory>
#include <functional>
#include "Market.h"
#include <boost/algorithm/string/replace.hpp>

class SinfarClient;

class RessourcesManager
{
    std::shared_ptr<Database> m_database;
    std::shared_ptr<orderentry::Market> m_market;
    std::shared_ptr<SinfarClient> m_client;
    
    void AddFilled(int PCId,std::string GoodsName,bool isBuy,int Quantity,int FilledPrice,std::function<void(std::string)> func);
public:
    RessourcesManager(std::shared_ptr<Database> database);
    void AddMarket(std::shared_ptr<orderentry::Market> market);
    void AddClient(std::shared_ptr<SinfarClient> sinfarClient);
    ~RessourcesManager();
    void ListInventory(std::function<void(std::string)> func,int PCId);
    int GetInventory(int PCId,std::string GoodsName);
    bool HasInventory(int PCId,std::string GoodsName);
    void AddInventory(std::function<void(std::string)> func,int PCId,std::string GoodsName,int quantity);
    void RemoveInventory(std::function<void(std::string)> func,int PCId,std::string GoodsName,int quantity);
    void ListGoods(std::function<void(std::string)> func);
    bool HasGoods(std::string Goodsname);
    void AddGoods(std::function<void(std::string)> func,std::string Goodsname,std::string GoodsDescription);
    bool HasAccount(int PCId);
    void AddAccount(std::function<void(std::string)> func,int PCId,bool Employee,int PlayerId,std::string name,std::string PlayerName,int fee);
    void DeleteAccount(int PCId);
    bool IsEmployee(int PCId);
    bool IsAdmin(int PCId);
    void UpdateMarket();
    bool OrderExists(std::string orderID);
    int addOrder(int PCId,std::string side,std::string symbol,int quantity,int price,int fee,int stopPrice=0,bool aon=false,bool ioc=false);
    bool replaceOrder(int PCId,int orderID,int dquantity,int newprice,std::function<void(std::string)> func);
    void UpdateOrderSeed(int orderSeed);
    void DebugListMarket(std::function<void(std::string)> func);
    void ListOrderMarket(std::function<void(std::string)> func,int PCId);
    void ListAllMarketPrices(std::function<void(std::string)> func);
    void ListPriceBook(std::function<void(std::string)> func,std::string GoodsName);
    void OrderInfo(std::string orderID,int& price,int& quantity,std::string& goodsName,bool& isBuy);
    void Command_InventoryAdd(int PCId,int PCIdTo,std::string GoodsName,int Quantity,std::function<void(std::string)> func) noexcept;
    void Command_InventoryRemove(int PCId,int PCIdTo,std::string GoodsName,int Quantity,std::function<void(std::string)> func) noexcept;
    void Command_ListInventory(int PCId,int PCIdTo,std::function<void(std::string)> func) noexcept;
    void Command_TradeBuy(int PCId,std::string GoodsName,int Quantity,int Price,std::function<void(std::string)> func) noexcept;
    void Command_TradeSell(int PCId,std::string GoodsName,int Quantity,int Price,std::function<void(std::string)> func) noexcept;
    void Command_Replace(int PCId,int orderID,int dQuantity,int Price,std::function<void(std::string)> func) noexcept;
    void Command_TradeListPrice(std::string GoodsName,std::function<void(std::string)> func) noexcept;
    void Command_AddAccount(std::string PlayerName,int PCId,int PCIdToAdd,bool EmployeeToAdd,int fee,std::function<void(std::string)> func) noexcept;
    void Command_NewGoods(int PCId,std::string GoodsName,std::string GoodsDescription,std::function<void(std::string)> func) noexcept;
    void Command_DeleteAccount(int PCId,int PCIdToDelete, std::function<void(std::string)> func) noexcept;
    void Command_Cancel(int PCId,int orderID, std::function<void(std::string)> func) noexcept;
    void Command_Cancel(int PCId,std::string GoodsName, std::function<void(std::string)> func) noexcept;
    void Command_Info(int PCId,std::function<void(std::string)> func) noexcept;
    void Command_TradeHistory(int PCId,std::function<void(std::string)> func) noexcept;
    
    void Command_shopAdd(int PCId,int quantity,std::function<void(std::string)> func) noexcept;
    void Command_shopRemove(int PCId,int quantity,std::function<void(std::string)> func) noexcept;
    void Command_shopInfo(int PCId,std::function<void(std::string)> func) noexcept;
    
    void shopAdd(int quantity,std::function<void(std::string)> func);
    void shopRemove(int quantity,std::function<void(std::string)> func);
    void shopInfo(std::function<void(std::string)> func);
    
    int shopGet(std::function<void(std::string)> func);
    int GetFee(int PCId);
                               
    void DeleteAccount(int PCId,std::function<void(std::string)> func);
    
    void on_accept(const orderentry::OrderPtr& order);
    void on_fill(const orderentry::OrderPtr& order,const orderentry::OrderPtr& matched_order,liquibook::book::Quantity fill_qty,liquibook::book::Cost fill_cost);
    void on_reject(const orderentry::OrderPtr& order, const char* reason);
    void on_cancel(const orderentry::OrderPtr& order);
    void on_cancel_reject(const orderentry::OrderPtr& order, const char* reason);
    void on_replace(const orderentry::OrderPtr& order,const int32_t& size_delta,liquibook::book::Price new_price);
    void on_replace_reject(const orderentry::OrderPtr& order, const char* reason);
};

inline std::string Escape(std::string in)
{
    boost::replace_all(in, "\"", "\"\"");
    boost::replace_all(in, "\'", "\'\'");
    return in;
}

#include "SinfarClient.h"
