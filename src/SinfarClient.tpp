#include "SinfarClient.h"
#include <boost/algorithm/string.hpp>    

SinfarClient::SinfarClient(std::shared_ptr<RessourcesManager> ressourceManager):m_ressourceManager(ressourceManager)
{
    char path_temp[]="/tmp/SinfarXXXXXXXXXXXXXXXXXXXX";
    std::string m_temp_dir=std::string(mkdtemp(path_temp));
    m_path=m_temp_dir+std::string("/bot_trader");
    Login();
}


SinfarClient::~SinfarClient()
{
    Logout();
}

void SinfarClient::Login()
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/login.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieJar(m_path.c_str()));
    curlpp::Forms form;
    form.push_back(new curlpp::FormParts::Content("player_name", "bot_trader"));
    form.push_back(new curlpp::FormParts::Content("password", "IcDarkUttOdTew6"));
    handle.setOpt(curlpp::Options::HttpPost(form));
    std::ostringstream os;
    os<<handle;
    std::string str=os.str();
    if(str.length()==0)
    {
    }
    else
    {
        throw std::runtime_error(std::string("impossible to login: ")+str);
    }
}

void SinfarClient::Logout()
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/logout.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieFile(m_path.c_str()));
    std::ostringstream os;
    os<<handle;
    std::string str=os.str();
    if(str.length()==0)
    {
    }
    else
    {
        throw std::runtime_error(std::string("impossible to logout: ")+str);
    }   
}

void SinfarClient::DoLoop()
{
    std::string messages_raw=PollMessage();
    if(messages_raw.length()>0)
    {
        json messages=json::parse(messages_raw)[0];
        for (json::iterator it = messages.begin(); it != messages.end(); ++it)
        {
            json message= *it;
            std::string channel=message["channel"];
            if(channel==std::string("4"))
            {
                std::string name=message["fromName"];
                std::string sPCId=message.value("fromPCId","-1");
                int PCId=std::stoi(sPCId);
                std::string sPlayerId=message.value("fromPlayerId","-1");
                int PlayerId=std::stoi(sPlayerId);
                std::string PlayerName=message["fromPlayerName"];
                std::string stringMessage=message["message"];
                if(PlayerId==SelfPlayerID())
                {
                    continue;
                }
                ParseTell(PCId,PlayerId,name,PlayerName,stringMessage);
            }
        }
    }
}

std::string SinfarClient::PollMessage()
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/getchat.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieFile(m_path.c_str()));
    std::ostringstream os;
    os<<handle;
    std::string str=os.str();
    return str;
}

void SinfarClient::ParseTell(int PCId,int PlayerId,std::string name,std::string PlayerName,std::string Message)
{
    if(m_ressourceManager->HasAccount(PCId))
    {
        {
            std::stringstream stream(Message);
            std::string command;
            stream>>command;
            boost::algorithm::to_lower(command);
            if(command==std::string("lg") || command==std::string("listgoods"))
            {
                std::function<void(std::string)> func=[&this,PlayerName](std::string message)
                {
                    SendMessage(PlayerName,message);
                };
                try
                {
                    m_ressourceManager->ListGoods(func);
                }
                catch(const std::exception& ex)
                {
                    SendMessage(PlayerName,ex.what());
                }
                catch (const std::string& ex)
                {
                    SendMessage(PlayerName,ex);
                }
            }
            else if(command==std::string("inventory") || command==std::string("i"))
            {
                std::string subcommand;
                stream>>subcommand;
                boost::algorithm::to_lower(subcommand);
                if(subcommand==std::string("add") || subcommand==std::string("a"))
                {
                    int PCIdTo;
                    stream>>PCIdTo;
                    std::string GoodsName;
                    stream>>GoodsName;
                    boost::algorithm::to_lower(GoodsName);
                    int Quantity;
                    stream>>Quantity;
                    if(Quantity>0&&PCIdTo>0 && m_ressourceManager->IsEmployee(PCId))
                    {
                        if(m_ressourceManager->HasGoods(GoodsName))
                        {
                            std::function<void(std::string)> func=[&this,PlayerName](std::string message)
                            {
                                SendMessage(PlayerName,message);
                            };
                            try
                            {
                                m_ressourceManager->AddInventory(func,PCIdTo,GoodsName,Quantity);
                            }
                            catch(const std::exception& ex)
                            {
                                SendMessage(PlayerName,ex.what());
                            }
                            catch (const std::string& ex)
                            {
                                SendMessage(PlayerName,ex);
                            }
                        }
                        else
                        {
                            SendMessage(PlayerName,std::string("GoodsName non existant: ")+GoodsName);
                        }
                    }
                }
                else if(subcommand==std::string("remove") || subcommand==std::string("r"))
                {
                    int PCIdTo;
                    stream>>PCIdTo;
                    std::string GoodsName;
                    stream>>GoodsName;
                    boost::algorithm::to_lower(GoodsName);
                    int Quantity;
                    stream>>Quantity;
                    if(Quantity>0&&PCIdTo>0 && m_ressourceManager->IsEmployee(PCId))
                    {
                        if(m_ressourceManager->HasGoods(GoodsName))
                        {
                            std::function<void(std::string)> func=[&this,PlayerName](std::string message)
                            {
                                SendMessage(PlayerName,message);
                            };
                            try
                            {
                            m_ressourceManager->RemoveInventory(func,PCIdTo,GoodsName,Quantity);
                            }
                            catch(const std::exception& ex)
                            {
                                SendMessage(PlayerName,ex.what());
                            }
                            catch (const std::string& ex)
                            {
                                SendMessage(PlayerName,ex);
                            }
                        }
                        else
                        {
                            SendMessage(PlayerName,std::string("GoodsName non existant: ")+GoodsName);
                        }
                    }
                }
                else if(subcommand==std::string("list") || subcommand==std::string("l"))
                {
                    int PCIdTo=-1;
                    stream>>PCIdTo;
                    if(PCIdTo==-1)
                    {
                        PCIdTo=PCId;
                    }
                    else if(!m_ressourceManager->IsEmployee(PCId))
                    {
                        return;
                    }
                    if(PCIdTo>0)
                    {
                        std::function<void(std::string)> func=[&PlayerName,&this](std::string message)
                        {
                            SendMessage(PlayerId,message);
                        };
                        try
                        {
                            m_ressourceManager->ListInventory(func,PCIdTo);
                        }
                        catch(const std::exception& ex)
                        {
                            SendMessage(PlayerName,ex.what());
                        }
                        catch (const std::string& ex)
                        {
                            SendMessage(PlayerName,ex);
                        }
                    }
                }
            }
            else if(command==std::string("trade") || command==std::string("t"))
            {
                std::string subcommand;
                stream>>subcommand;
                boost::algorithm::to_lower(subcommand);
                if(subcommand==std::string("buy") || subcommand==std::string("b"))
                {
                    std::string GoodsName;
                    stream>>GoodsName;
                    boost::algorithm::to_lower(GoodsName);
                    int Quantity;
                    stream>>Quantity;
                    int Price;
                    stream>>Price;
                    if(Quantity>0&&Price>0)
                    {
                        try
                        {
                        if(m_ressourceManager->HasGoods(GoodsName))
                        {
                            int goldAvailable=m_ressourceManager->GetInventory(PCId,"gold");
                            if(goldAvailable>=Price)
                            {
                                int idOrder=m_market->addOrder("BUY",GoodsName,Quantity,Price);
                                if(idOrder>=0)
                                {
                                    m_ressourceManager->AddGoodsToBuy(PCId,GoodsName,idOrder,Quantity,Price);
                                    SendMessage(PlayerName,std::string("Buy Order submitted for Goods=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity)+std::string(" Price=")+std::to_string(Price)+std::string(" orderID=")+std::to_string(idOrder));
                                }
                            }
                            else
                            {
                                SendMessage(PlayerName,std::string("You do not have enought gold: ")+GoodsName);
                            }
                        }
                        else
                        {
                            SendMessage(PlayerName,std::string("GoodsName non existant: ")+GoodsName);
                        }
                        }
                        catch(const std::exception& ex)
                        {
                            SendMessage(PlayerName,ex.what());
                        }
                        catch (const std::string& ex)
                        {
                            SendMessage(PlayerName,ex);
                        }
                    }
                }
                else if(subcommand==std::string("sell") || subcommand==std::string("s"))
                {
                    std::string GoodsName;
                    stream>>GoodsName;
                    boost::algorithm::to_lower(GoodsName);
                    int Quantity;
                    stream>>Quantity;
                    int Price;
                    stream>>Price;
                    if(Quantity>0&&Price>0)
                    {
                        try
                        {
                        if(m_ressourceManager->HasGoods(GoodsName))
                        {
                            int goodsAvailable=m_ressourceManager->GetInventory(PCId,GoodsName);
                            if(goodsAvailable>=Quantity)
                            {
                                int idOrder=m_market->addOrder("SELL",GoodsName,Quantity,Price);
                                if(idOrder>=0)
                                {
                                    m_ressourceManager->AddGoodsToSell(PCId,GoodsName,idOrder,Quantity,Price);
                                    SendMessage(PlayerName,std::string("Sell Order submitted for Goods=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity)+std::string(" Price=")+std::to_string(Price)+std::string(" orderID=")+std::to_string(idOrder));
                                }
                            }
                            else
                            {
                                SendMessage(PlayerName,std::string("You do not have enought Goods: ")+GoodsName);
                            }
                        }
                        else
                        {
                            SendMessage(PlayerName,std::string("GoodsName non existant: ")+GoodsName);
                        }
                        }
                        catch(const std::exception& ex)
                        {
                            SendMessage(PlayerName,ex.what());
                        }
                        catch (const std::string& ex)
                        {
                            SendMessage(PlayerName,ex);
                        }
                    }
                }
            }
        }
        
        if(m_ressourceManager->IsEmployee(PCId))
        {
            std::stringstream stream(Message);
            std::string command;
            stream>>command;
            boost::algorithm::to_lower(command);
            if(command==std::string("a") || command==std::string("add"))
            {
                int PCIdToAdd=-1;
                bool EmployeeToAdd=false;
                stream>>PCIdToAdd;
                if(PCIdToAdd>0)
                {
                    if(m_ressourceManager->IsAdmin(PCId))
                    {
                        std::string Employee;
                        stream>>Employee;
                        if(Employee==std::string("e"))
                        {
                            EmployeeToAdd=true;
                        }
                    }
                    AddAccount(PlayerName,PCIdToAdd,EmployeeToAdd);
                }
            }
            else if(command==std::string("n") || command==std::string("new"))
            {
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                std::string GoodsDescription;
                std::getline(stream,GoodsDescription);
                std::function<void(std::string)> func=[&PlayerName,&this](std::string message)
                {
                    SendMessage(PlayerId,message);
                };
                try
                {
                    m_ressourceManager->AddGoods(func,GoodsName,GoodsDescription);
                }
                catch(const std::exception& ex)
                {
                    SendMessage(PlayerName,ex.what());
                }
                catch (const std::string& ex)
                {
                    SendMessage(PlayerName,ex);
                }
            }
        }
        if(IsAdmin(PCId))
        {
            std::stringstream stream(Message);
            std::string command;
            stream>>command;
            boost::algorithm::to_lower(command);
            if(command==std::string("d") || command==std::string("delete"))
            {
                int PCIdToDelete=-1;
                stream>>PCIdToDelete;
                if(PCIdToDelete>0)
                {
                    DeleteAccount(PlayerName,PCIdToDelete);
                }
            }
            else
            {
            }
        }
    }
    else
    {
        try
        {
            SendMessage(PlayerName,std::string("You do not have an account, please contact \"Alchemist\" or a member of the house of the Purple Alchemist of Durakiss"));
        }
        catch(...)
        {
        }
    }
}

void SinfarClient::AddAccount(std::string AdderName,int PCId,bool Employee)
{
    try
    {
        int PlayerId;
        std::string name;
        std::string PlayerName;
        int permission_level=0;
        GetPCInformation(PCId,PlayerId,name,PlayerName);
        std::function<void(std::string)> func=[&this,AdderName](std::string message)
        {
            SendMessage(AdderName,message);
        };
        m_ressourceManager->AddAccount(func, PCId, Employee, PlayerId, name, PlayerName)
    }
    catch(const std::exception& ex)
    {
        SendMessage(AdderName,ex.what());
    }
    catch (const std::string& ex)
    {
        SendMessage(AdderName,ex);
    }
}

void SinfarClient::DeleteAccount(std::string DeleterName,int PCId)
{
    try
    {
        int PlayerId;
        std::string name;
        std::string PlayerName;
        int permission_level=0;
        GetPCInformation(PCId,PlayerId,name,PlayerName);
        if(m_ressourceManager->HasAccount(PCId))
        {
            m_ressourceManager->DeleteAccount(PCId);
        }
        else
        {
            SendMessage(DeleterName,std::string("No Account found: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName);
        }
        SendMessage(DeleterName,std::string("Delete sucessful: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName);
    }
    catch(const std::exception& ex)
    {
        SendMessage(DeleterName,ex.what());
    }
    catch (const std::string& ex)
    {
        SendMessage(DeleterName,ex);
    }
}

void SinfarClient::SendMessage(std::string PlayerName,std::string Message)
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/sendchat.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieFile(m_path.c_str()));
    curlpp::Forms form;
    std::string message_=std::string("/tp \"")+PlayerName+std::string("\" ")+Message;
    form.push_back(new curlpp::FormParts::Content("chat-message",message_));
    form.push_back(new curlpp::FormParts::Content("channel","tell"));
    handle.setOpt(curlpp::Options::HttpPost(form));
    std::ostringstream os;
    os<<handle;
    std::string str=os.str();
    if(str.length()==0)
    {
    }
    else
    {
        throw std::runtime_error(std::string("impossible to SendMessage: ")+str);
    }
}

int SinfarClient::SelfPlayerID() const
{
    return 46191;
}

void SinfarClient::GetPCInformation(int PCId,int& PlayerId,std::string& name,std::string& PlayerName)
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/getonlineplayers.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieFile(m_path.c_str()));
    std::ostringstream os;
    os<<handle;
    std::string str=os.str();
    if(str.length()==0)
    {
        throw std::runtime_error(std::string("impossible to getonlineplayers: ")+str);
    }
    else
    {
        json list_player=json::parse(str);
        for (json::iterator it = list_player.begin(); it != list_player.end(); ++it)
        {
            json current_player= *it;
            int curPCId=std::stoi(current_player.value("pcId","-1"));
            if(curPCId==PCId)
            {
                PlayerId=std::stoi(current_player.value("playerId","-1"));
                name=current_player["pcName"];
                PlayerName=current_player["playerName"];
            }
        }
    }
}
