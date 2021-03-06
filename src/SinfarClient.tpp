#include "SinfarClient.h"
#include <boost/algorithm/string.hpp>
#include "Color.h"

inline SinfarClient::SinfarClient(std::shared_ptr<RessourcesManager> ressourceManager):m_ressourceManager(ressourceManager)
{
    char path_temp[]="/tmp/SinfarXXXXXXXXXXXXXXXXXXXX";
    std::string m_temp_dir=std::string(mkdtemp(path_temp));
    m_path=m_temp_dir+std::string("/bot_trader");
    Login();
}


inline SinfarClient::~SinfarClient()
{
    Logout();
}

inline void SinfarClient::Login()
{
    curlpp::Easy handle;
    std::string url=std::string("https://nwn.sinfar.net/login.php?nocache=")+std::to_string(std::time(0));
    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CookieJar(m_path.c_str()));
    curlpp::Forms form;
    form.push_back(new curlpp::FormParts::Content("player_name", "bot"));
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

inline void SinfarClient::Logout()
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

inline void SinfarClient::DoLoop()
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

inline std::string SinfarClient::PollMessage()
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

inline void SinfarClient::ParseTell(int PCId,int PlayerId,std::string name,std::string PlayerName,std::string Message)
{
    std::cout<<"Parsing PCId="<<PCId<<" PlayerId="<<PlayerId<<" Name="<<name<<" PlayerName="<<PlayerName<<" Message="<<Message<<std::endl;
    std::function<void(std::string)> func=[this,PlayerName](std::string message)
    {
        SendMessage(PlayerName,message);
    };
    if(m_ressourceManager->HasAccount(PCId))
    {
        {
            std::stringstream stream(Message);
            std::string command;
            stream>>command;
            boost::algorithm::to_lower(command);
            if(command==std::string("lg") || command==std::string("listgoods"))
            {
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
            else if(command==std::string("h") || command==std::string("help"))
            {
                std::string text=std::string("The following basic command are possible:\n")
                                +ColorRed("listgoods")+std::string(": List the goods that can be traded\n")
                                +ColorRed("help")+std::string(": Give this help message\n")
                                +ColorRed("info")+std::string(": Give details on your profile\n")
                                +ColorRed("list_i")+std::string(": List all item in your inventory\n")
                                +ColorRed("buy <quantity> <goodsname> <price>")+std::string(": Add a buy order\n")
                                +ColorRed("sell <quantity> <goodsname> <price>")+std::string(": Add a sell order\n")
                                +ColorRed("bid <quantity> <goodsname> <price>")+std::string(": Add a bid order. This replace previous order for the same goods.\n")
                                +ColorRed("replace <order id> <delta quantity> <new price>")+std::string(": Replace an order\n")
                                +ColorRed("cancel <goodsname>")+std::string(": Cancel all order for goodsname. If missing, cancel all order.\n")
                                +ColorRed("cancel_id <order id>")+std::string(": Cancel an order\n")
                                +ColorRed("history")+std::string(": Show the history of your trading\n")
                                +ColorRed("list")+std::string(": List all pending order\n")
                                +ColorRed("price <goodsname>")+std::string(": List the price of current goods on market. If goodsname missing show all prices.\n");
                                if(m_ressourceManager->IsEmployee(PCId))
                                {
                                    text+=ColorRed("new <shortname> <description>")+std::string(": Define a new goods.\n")
                                    +ColorRed("inventory add <PCID> <goodsname> <quantity>")+std::string(": Add goods into inventory.\n")
                                    +ColorRed("inventory remove <PCID> <goodsname> <quantity>")+std::string(": Remove goods from inventory.\n")
                                    +ColorRed("list_i <PCID>")+std::string(": List all item in player inventory.\n");

                                }
                
                                std::cout<<"message "<<text<<std::endl;
                SendMessage(PlayerName,text);
            }
            else if(command==std::string("info") || command==std::string("in"))
            {
                m_ressourceManager->Command_Info(PCId,func);
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
                    m_ressourceManager->Command_InventoryAdd(PCId,PCIdTo,GoodsName,Quantity,func);
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
                    m_ressourceManager->Command_InventoryRemove(PCId,PCIdTo,GoodsName,Quantity,func);
                }
                else if(subcommand==std::string("list") || subcommand==std::string("l"))
                {
                    int PCIdTo=-1;
                    stream>>PCIdTo;
                    m_ressourceManager->Command_ListInventory(PCId,PCIdTo,func);
                }
            }
            else if(command==std::string("list_i") || command==std::string("li"))
            {
                int PCIdTo=-1;
                stream>>PCIdTo;
                m_ressourceManager->Command_ListInventory(PCId,PCIdTo,func);
            }
            else if(command==std::string("buy") || command==std::string("b"))
            {
                int Quantity;
                stream>>Quantity;
                std::string GoodsName;
                boost::algorithm::to_lower(GoodsName);
                stream>>GoodsName;
                int Price;
                stream>>Price;
                m_ressourceManager->Command_TradeBuy(PCId,GoodsName,Quantity,Price,func);
            }
            else if(command==std::string("sell") || command==std::string("s"))
            {
                int Quantity=0;
                stream>>Quantity;
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                int Price=0;
                stream>>Price;
                m_ressourceManager->Command_TradeSell(PCId,GoodsName,Quantity,Price,func);
            }
            else if(command==std::string("bid"))
            {
                int Quantity=0;
                stream>>Quantity;
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                int Price=0;
                stream>>Price;
                m_ressourceManager->Command_Bid(PCId,GoodsName,Quantity,Price,func);
            }
            else if(command==std::string("replace") || command==std::string("r"))
            {
                int orderID=0;
                stream>>orderID;
                int dQuantity=0;
                stream>>dQuantity;
                int Price=0;
                stream>>Price;
                m_ressourceManager->Command_Replace(PCId,orderID,dQuantity,Price,func);
            }
            else if(command==std::string("cancel_id") || command==std::string("ci"))
            {
                int orderID=0;
                stream>>orderID;
                m_ressourceManager->Command_Cancel(PCId,orderID,func);
            }
            else if(command==std::string("cancel") || command==std::string("c"))
            {
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                m_ressourceManager->Command_Cancel(PCId,GoodsName,func);
            }
            else if(command==std::string("history") || command==std::string("h"))
            {
                m_ressourceManager->Command_TradeHistory(PCId,func);
            }
                
                /*else if(subcommand==std::string("debuglist") || subcommand==std::string("dl"))
                {
                    std::function<void(std::string)> func=[&PlayerName,this](std::string message)
                    {
                        SendMessage(PlayerName,message);
                    };
                    m_ressourceManager->DebugListMarket(func);
                }*/
            else if(command==std::string("list") || command==std::string("l"))
            {
                m_ressourceManager->ListOrderMarket(func,PCId);
            }
            else if(command==std::string("price") || command==std::string("p"))
            {
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                m_ressourceManager->Command_TradeListPrice(GoodsName,func);
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
                    int fee=5;
                    stream>>fee;
                    if(m_ressourceManager->IsAdmin(PCId))
                    {
                        std::string Employee;
                        stream>>Employee;
                        if(Employee==std::string("e"))
                        {
                            EmployeeToAdd=true;
                        }
                    }
                    m_ressourceManager->Command_AddAccount(PlayerName,PCId,PCIdToAdd,EmployeeToAdd,fee,func);
                }
            }
            else if(command==std::string("n") || command==std::string("new"))
            {
                std::string GoodsName;
                stream>>GoodsName;
                boost::algorithm::to_lower(GoodsName);
                std::string GoodsDescription;
                std::getline(stream,GoodsDescription);
                m_ressourceManager->Command_NewGoods(PCId,GoodsName,GoodsDescription,func);
            }
        }
        if(m_ressourceManager->IsAdmin(PCId))
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
                    m_ressourceManager->Command_DeleteAccount(PCId,PCIdToDelete,func);
                }
            }
            else if(command==std::string("shop"))
            {
                std::string subcommand;
                stream>>subcommand;
                boost::algorithm::to_lower(subcommand);
                if(subcommand==std::string("add") || subcommand==std::string("a"))
                {
                    int quantity=0;
                    stream>>quantity;
                    m_ressourceManager->Command_shopAdd(PCId,quantity,func);
                }
                else if(subcommand==std::string("remove") || subcommand==std::string("r"))
                {
                    int quantity=0;
                    stream>>quantity;
                    m_ressourceManager->Command_shopRemove(PCId,quantity,func);
                }
                else if(subcommand==std::string("info") || subcommand==std::string("i"))
                {
                    int quantity=0;
                    stream>>quantity;
                    m_ressourceManager->Command_shopInfo(PCId,func);
                }
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

inline void SinfarClient::AddAccount(std::string AdderName,int PCId,int fee,bool Employee)
{
    try
    {
        int PlayerId;
        std::string name;
        std::string PlayerName;
        int permission_level=0;
        GetPCInformation(PCId,PlayerId,name,PlayerName);
        std::function<void(std::string)> func=[this,AdderName](std::string message)
        {
            SendMessage(AdderName,message);
        };
        m_ressourceManager->AddAccount(func, PCId, Employee, PlayerId, name, PlayerName,fee);
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

inline void SinfarClient::DeleteAccount(std::string DeleterName,int PCId)
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

inline void SinfarClient::SendMessage(std::string PlayerName,std::string Message) const
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

inline int SinfarClient::SelfPlayerID() const
{
    return 46459;
}

inline void SinfarClient::GetPCInformation(int PCId,int& PlayerId,std::string& name,std::string& PlayerName) const
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

inline std::function<void(std::string)> SinfarClient::GetMessager(int PCId) const
{
    if(m_messager.count(PCId)>0)
    {
        return m_messager.at(PCId);
    }
    else
    {
        int PlayerId;
        std::string name;
        std::string PlayerName;
        GetPCInformation(PCId,PlayerId,name,PlayerName);
        std::function<void(std::string)> func=[PlayerName,this](std::string message)
        {
            SendMessage(PlayerName,message);
        };
        m_messager[PCId]=func;
        return func;
    }
}
