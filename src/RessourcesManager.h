#pragma once
#include "Database.h"
#include <iostream>
#include <memory>
#include <functional>
#include "Market.h"

class RessourcesManager
{
    std::shared_ptr<Database> m_database;
    std::shared_ptr<Market> m_market;
public:
    RessourcesManager(std::shared_ptr<Database> database);
    void AddMarket(std::shared_ptr<Market> market);
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
};
