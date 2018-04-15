#pragma once

#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Types.hpp>
#include <curlpp/Form.hpp>

#include "json.hpp"
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include "Market.h"
#include "RessourcesManager.h"

using json = nlohmann::json;

class SinfarClient
{
    std::string m_path;
    std::shared_ptr<RessourcesManager> m_ressourceManager;
    mutable std::map<int,std::function<void(std::string)>> m_messager;
public:
    SinfarClient(std::shared_ptr<RessourcesManager> database);
    ~SinfarClient();
    void DoLoop();
    void SendMessage(std::string PlayerName,std::string Message) const;
    void AddAccount(std::string AdderName,int PCId,bool Employee);
    void DeleteAccount(std::string DeleterName,int PCId);
    void GetPCInformation(int PCId,int& PlayerId,std::string& name,std::string& PlayerName) const;
    std::function<void(std::string)> GetMessager(int PCId) const;
private:
    void Login();
    void Logout();
    std::string PollMessage();
    void ParseTell(int PCId,int PlayerId,std::string name,std::string PlayerName,std::string Message);
    int SelfPlayerID() const;
};

#include "SinfarClient.tpp"
