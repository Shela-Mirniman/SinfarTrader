#pragma once
#include "Database.h"
#include <iostream>
#include <memory>
#include <functional>
#include "Market.h"

class RessourcesManager
{
    std::shared_ptr<Database> m_database;
    std::shared_ptr<orderentry::Market> m_market;
public:
    RessourcesManager(std::shared_ptr<Database> database);
    void AddMarket(std::shared_ptr<orderentry::Market> market);
    ~RessourcesManager();
    void ListInventory(std::function<void(std::string)> func,int PCId);
    void AddGoodsToSell(int PCId,std::string GoodsName,int orderID,int Quantity,int price);
    void AddGoodsToBuy(int PCId,std::string GoodsName,int orderID,int Quantity,int price);
    int GetInventory(int PCId,std::string GoodsName);
    bool HasInventory(int PCId,std::string GoodsName);
    void AddInventory(std::function<void(std::string)> func,int PCId,std::string GoodsName,int quantity);
    void RemoveInventory(std::function<void(std::string)> func,int PCId,std::string GoodsName,int quantity);
    void ListGoods(std::function<void(std::string)> func);
    bool HasGoods(std::string Goodsname);
    void AddGoods(std::function<void(std::string)> func,std::string Goodsname,std::string GoodsDescription);
    bool HasAccount(int PCId);
    void AddAccount(std::function<void(std::string)> func,int PCId,bool Employee,int PlayerId,std::string name,std::string PlayerName);
    void DeleteAccount(int PCId);
    bool IsEmployee(int PCId);
    bool IsAdmin(int PCId);
    void UpdateMarket();
    bool OrderExists(std::string orderID);
    int addOrder(int PCId,std::string side,std::string symbol,int quantity,int price,int stopPrice=0,bool aon=false,bool ioc=false);
    void on_accept(const orderentry::OrderPtr& order);
    void on_fill(const orderentry::OrderPtr& order,const orderentry::OrderPtr& matched_order,liquibook::book::Quantity fill_qty,liquibook::book::Cost fill_cost);
    void UpdateOrderSeed(int orderSeed);
};
