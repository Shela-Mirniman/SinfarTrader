#pragma once

#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Types.hpp>
#include <curlpp/Form.hpp>

#include "json.hpp"
#include "Database.h"
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include "Market.h"

using json = nlohmann::json;

class SinfarClient
{
    std::string m_path;
    std::shared_ptr<Database> m_database;
    std::shared_ptr<orderentry::Market> m_market;
public:
    SinfarClient(std::shared_ptr<Database> database,std::shared_ptr<orderentry::Market> market);
    ~SinfarClient();
    void DoLoop();
private:
    void UpdateMarket();
    void Login();
    void Logout();
    std::string PollMessage();
    void ParseTell(int PCId,int PlayerId,std::string name,std::string PlayerName,std::string Message);
    bool HasAccount(int PCId);
    bool IsEmployee(int PCId);
    bool IsAdmin(int PCId);
    int SelfPlayerID() const;
    void SendMessage(std::string PlayerName,std::string Message);
    void AddAccount(std::string AdderName,int PCId,bool Employee);
    void DeleteAccount(std::string DeleterName,int PCId);
    void GetPCInformation(int PCId,int& PlayerId,std::string& name,std::string& PlayerName);
    void AddGoods(std::string AdderName,std::string Goodsname,std::string GoodsDescription);
    bool HasGoods(std::string Goodsname);
    void ListGoods(std::string ListerName);
    void AddInventory(std::string AdderName,int PCId,std::string GoodsName,int quantity);
    void RemoveInventory(std::string AdderName,int PCId,std::string GoodsName,int quantity);
    bool HasInventory(int PCId,std::string GoodsName);
    int GetInventory(int PCId,std::string GoodsName);
    void AddGoodsToBuy(int PCId,std::string GoodsName,int orderID,int Quantity,int price);
    void AddGoodsToSell(int PCId,std::string GoodsName,int orderID,int Quantity,int price);
    void ListInventory(std::string ListerName,int PCId);
};

#include "SinfarClient.tpp"
