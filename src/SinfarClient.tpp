#include "SinfarClient.h"
#include <boost/algorithm/string.hpp>    

SinfarClient::SinfarClient(std::shared_ptr<Database> database,std::shared_ptr<orderentry::Market> market):m_database(database),m_market(market)
{
    char path_temp[]="/tmp/SinfarXXXXXXXXXXXXXXXXXXXX";
    std::string m_temp_dir=std::string(mkdtemp(path_temp));
    m_path=m_temp_dir+std::string("/bot_trader");
    UpdateMarket();
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

bool SinfarClient::HasAccount(int PCId)
{
    bool banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<bool*>(data)=count>0;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT count(*) FROM account where PCId=")+std::to_string(PCId);
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

bool SinfarClient::IsEmployee(int PCId) 
{
    bool banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<bool*>(data)=count>0;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT count(*) FROM account where PCId=")+std::to_string(PCId)+std::string(" and permission_level>4");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

bool SinfarClient::IsAdmin(int PCId) 
{
    bool banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<bool*>(data)=count>0;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT count(*) FROM account where PCId=")+std::to_string(PCId)+std::string(" and permission_level>9");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

void SinfarClient::ParseTell(int PCId,int PlayerId,std::string name,std::string PlayerName,std::string Message)
{
    if(HasAccount(PCId))
    {
        {
            std::stringstream stream(Message);
            std::string command;
            stream>>command;
            boost::algorithm::to_lower(command);
            if(command==std::string("lg") || command==std::string("listgoods"))
            {
                ListGoods(PlayerName);
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
                    if(Quantity>0&&PCIdTo>0 && IsEmployee(PCId))
                    {
                        if(HasGoods(GoodsName))
                        {
                            AddInventory(PlayerName,PCIdTo,GoodsName,Quantity);
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
                    if(Quantity>0&&PCIdTo>0 && IsEmployee(PCId))
                    {
                        if(HasGoods(GoodsName))
                        {
                            RemoveInventory(PlayerName,PCIdTo,GoodsName,Quantity);
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
                    else if(!IsEmployee(PCId))
                    {
                        return;
                    }
                    if(PCIdTo>0)
                    {
                        ListInventory(PlayerName,PCIdTo);
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
                        if(HasGoods(GoodsName))
                        {
                            int goldAvailable=GetInventory(PCId,"gold");
                            if(goldAvailable>=Price)
                            {
                                int idOrder=m_market->addOrder("BUY",GoodsName,Quantity,Price);
                                if(idOrder>=0)
                                {
                                    AddGoodsToBuy(PCId,GoodsName,idOrder,Quantity,Price);
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
                        if(HasGoods(GoodsName))
                        {
                            int goodsAvailable=GetInventory(PCId,GoodsName);
                            if(goodsAvailable>=Quantity)
                            {
                                int idOrder=m_market->addOrder("SELL",GoodsName,Quantity,Price);
                                if(idOrder>=0)
                                {
                                    AddGoodsToSell(PCId,GoodsName,idOrder,Quantity,Price);
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
        
        if(IsEmployee(PCId))
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
                    if(IsAdmin(PCId))
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
                AddGoods(PlayerName,GoodsName,GoodsDescription);
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
        if(Employee)
        {
            permission_level=5;
        }
        if(HasAccount(PCId))
        {
            char *errorMessage;
            std::string sql=std::string("UPDATE account set PlayerId=")+std::to_string(PlayerId)+std::string(" PlayerName=")+PlayerName+std::string(" name=")+name+std::string(" permission_level=")+std::to_string(permission_level)+std::string(" where PCId=")+std::to_string(PCId);
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        else
        {
            char *errorMessage;
            std::string sql=std::string("INSERT INTO account VALUES(")+std::to_string(PCId)+std::string(",")+std::to_string(PlayerId)+std::string(",'")+PlayerName+std::string("','")+name+std::string("',")       +std::to_string(permission_level)+std::string(")");
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        SendMessage(AdderName,std::string("Add sucessful: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName+std::string(" Permission_Level=")+std::to_string(permission_level));
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
        if(HasAccount(PCId))
        {
            char *errorMessage;
            std::string sql=std::string("DELETE from account ")+std::string(" where PCId=")+std::to_string(PCId);
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
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

void SinfarClient::AddGoods(std::string AdderName,std::string Goodsname,std::string GoodsDescription)
{
    try
    {
        if(HasGoods(Goodsname))
        {
            char *errorMessage;
            std::string sql=std::string("UPDATE goodsList set goodsDescription='")+GoodsDescription+std::string("' where goodsName='")+Goodsname+std::string("'");
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        else
        {
            char *errorMessage;
            std::string sql=std::string("INSERT INTO goodsList VALUES('")+Goodsname+std::string("','")+GoodsDescription+std::string("')");
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
            if(!m_market->symbolIsDefined(Goodsname))
            {
                m_market->addBook(Goodsname);
            }
        }
        SendMessage(AdderName,std::string("Goods added sucessfuly: Goodsname=")+Goodsname+std::string(" GoodsDescription=")+GoodsDescription);
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

bool SinfarClient::HasGoods(std::string GoodsName)
{
    bool banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<bool*>(data)=count>0;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT count(*) FROM goodsList where goodsName='")+GoodsName+std::string("'");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

void SinfarClient::ListGoods(std::string ListerName)
{
    try{
        std::string message("Lists of Goods:\n");
        auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==2)
            {
                *static_cast<std::string*>(data)+=std::string(columnText[0])+std::string(" ")+std::string(columnText[1])+std::string("\n");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT * FROM goodsList");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        SendMessage(ListerName,message);
    }
    catch(const std::exception& ex)
    {
        SendMessage(ListerName,ex.what());
    }
    catch (const std::string& ex)
    {
        SendMessage(ListerName,ex);
    }
}

void SinfarClient::AddInventory(std::string AdderName,int PCIdTo,std::string GoodsName,int Quantity)
{
    try
    {
    if(HasInventory(PCIdTo,GoodsName))
    {
        Quantity+=GetInventory(PCIdTo,GoodsName);
        char *errorMessage;
        std::string sql=std::string("UPDATE inventory set quantity=")+std::to_string(Quantity)+std::string(" where PCId=")+std::to_string(PCIdTo)+std::string(" and goodsName='")+GoodsName+std::string("'");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
                throw std::runtime_error(errorMessage);
        }
    }
    else
    {
        char *errorMessage;
        std::string sql=std::string("INSERT INTO inventory VALUES(")+std::to_string(PCIdTo)+std::string(",'")+GoodsName+std::string("',")+std::to_string(Quantity)+std::string(")");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
    SendMessage(AdderName,std::string("Updated inventory of PCId=")+std::to_string(PCIdTo)+std::string(" GoodsName=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity));
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

void SinfarClient::RemoveInventory(std::string AdderName,int PCIdTo,std::string GoodsName,int Quantity)
{
    try
    {
        if(HasInventory(PCIdTo,GoodsName))
        {
            Quantity=GetInventory(PCIdTo,GoodsName)-Quantity;
            if(Quantity>0)
            {
                char *errorMessage;
                std::string sql=std::string("UPDATE inventory set quantity=")+std::to_string(Quantity)+std::string(" where PCId=")+std::to_string(PCIdTo)+std::string(" and goodsName='")+GoodsName+std::string("'");
                if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
                {
                    throw std::runtime_error(errorMessage);
                }
            }
            else
            {
                throw std::runtime_error(std::string("Quantity going negative ")+std::to_string(Quantity));
            }
        }
        else
        {
            throw std::runtime_error(std::string("Quantity going negative ")+std::to_string(Quantity));
        }
        SendMessage(AdderName,std::string("Updated inventory of PCId=")+std::to_string(PCIdTo)+std::string(" GoodsName=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity));
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

bool SinfarClient::HasInventory(int PCIdTo,std::string GoodsName)
{
    bool banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<bool*>(data)=count>0;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT count(*) FROM inventory where PCId=")+std::to_string(PCIdTo)+std::string(" and goodsName='")+GoodsName+std::string("'");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

int SinfarClient::GetInventory(int PCIdTo,std::string GoodsName)
{
    int banswer;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn>0)
        {
            int count=std::stoi(columnText[0]);
            *static_cast<int*>(data)=count;
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT quantity FROM inventory where PCId=")+std::to_string(PCIdTo)+std::string(" and goodsName='")+GoodsName+std::string("'");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

void SinfarClient::ListInventory(std::string ListerName,int PCId)
{
    try{
        std::string message("Lists of Goods in Invetory:\n");
        auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==2)
            {
                *static_cast<std::string*>(data)+=std::string(columnText[0])+std::string(" ")+std::string(columnText[1])+std::string("\n");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT goodsName,quantity FROM inventory where PCId=")+std::to_string(PCId);
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        SendMessage(ListerName,message);
    }
    catch(const std::exception& ex)
    {
        SendMessage(ListerName,ex.what());
    }
    catch (const std::string& ex)
    {
        SendMessage(ListerName,ex);
    }
}

void SinfarClient::UpdateMarket()
{
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn==1)
        {
            auto market=static_cast<SinfarClient*>(data)->m_market;
            if(!market->symbolIsDefined(columnText[0]))
            {
                market->addBook(columnText[0]);
            }
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT goodsName FROM goodsList");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,this,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}

void SinfarClient::AddGoodsToBuy(int PCId,std::string GoodsName,int orderID,int Quantity,int Price)
{
    int UpdateQuantity=GetInventory(PCId,"gold")-Price;
    if(UpdateQuantity>0)
    {
        char *errorMessage;
        std::string sql=std::string("UPDATE inventory set quantity=")+std::to_string(UpdateQuantity)+std::string(" where PCId=")+std::to_string(PCId)+std::string(" and goodsName='gold' ; INSERT INTO inventory_trading VALUES(")+std::to_string(orderID)+std::string(",0")+std::string(",'")+GoodsName+std::string("',")+std::to_string(PCId)+std::string(",")+std::to_string(Quantity)+std::string(",")+std::to_string(Price)+std::string(")");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
    else
    {
        throw std::runtime_error(std::string("Quantity going negative ")+std::to_string(Quantity));
    }
}

void SinfarClient::AddGoodsToSell(int PCId,std::string GoodsName,int orderID,int Quantity,int Price)
{
    int UpdateQuantity=GetInventory(PCId,GoodsName)-Quantity;
    if(UpdateQuantity>0)
    {
        char *errorMessage;
        std::string sql=std::string("UPDATE inventory set quantity=")+std::to_string(UpdateQuantity)+std::string(" where PCId=")+std::to_string(PCId)+std::string(" and goodsName='")+GoodsName+std::string("' ; INSERT INTO inventory_trading VALUES(")+std::to_string(orderID)+std::string(",1")+std::string(",'")+GoodsName+std::string("',")+std::to_string(PCId)+std::string(",")+std::to_string(Quantity)+std::string(",")+std::to_string(Price)+std::string(")");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
    else
    {
        throw std::runtime_error(std::string("Quantity going negative ")+std::to_string(Quantity));
    }
}
