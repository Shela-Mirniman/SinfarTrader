#include "RessourcesManager.h"

RessourcesManager::RessourcesManager(std::shared_ptr<Database> database):m_database(database)
{
}

RessourcesManager::~RessourcesManager()
{
}

void RessourcesManager::AddMarket(std::shared_ptr<orderentry::Market> market)
{
    m_market=market;
}

void RessourcesManager::ListInventory(std::function<void(std::string)> func,int PCId)
{
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
    func(message);
}

int RessourcesManager::GetInventory(int PCIdTo,std::string GoodsName)
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

bool RessourcesManager::HasInventory(int PCIdTo,std::string GoodsName)
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

void RessourcesManager::AddInventory(std::function<void(std::string)> func,int PCIdTo,std::string GoodsName,int Quantity)
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
    func(std::string("Updated inventory of PCId=")+std::to_string(PCIdTo)+std::string(" GoodsName=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity));
}

void RessourcesManager::RemoveInventory(std::function<void(std::string)> func,int PCIdTo,std::string GoodsName,int Quantity)
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
        func(std::string("Updated inventory of PCId=")+std::to_string(PCIdTo)+std::string(" GoodsName=")+GoodsName+std::string(" Quantity=")+std::to_string(Quantity));
}


void RessourcesManager::ListGoods(std::function<void(std::string)> func)
{
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
        func(message);
}

bool RessourcesManager::HasGoods(std::string GoodsName)
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

void RessourcesManager::AddGoods(std::function<void(std::string)> func,std::string Goodsname,std::string GoodsDescription)
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
        func(std::string("Goods added sucessfuly: Goodsname=")+Goodsname+std::string(" GoodsDescription=")+GoodsDescription);
}

bool RessourcesManager::HasAccount(int PCId)
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

void RessourcesManager::AddAccount(std::function<void(std::string)> func,int PCId,bool Employee,int PlayerId,std::string name,std::string PlayerName)
{
        int permission_level=0;
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
        func(std::string("Add sucessful: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName+std::string(" Permission_Level=")+std::to_string(permission_level));
}

void RessourcesManager::DeleteAccount(int PCId)
{
    char *errorMessage;
    std::string sql=std::string("DELETE from account ")+std::string(" where PCId=")+std::to_string(PCId);
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}

bool RessourcesManager::IsEmployee(int PCId) 
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

bool RessourcesManager::IsAdmin(int PCId) 
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

void RessourcesManager::UpdateMarket()
{
{
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn==1)
        {
            auto market=static_cast<RessourcesManager*>(data)->m_market;
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
{
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn==9)
        {
            auto market=static_cast<RessourcesManager*>(data)->m_market;
            std::string order("SELL");
            if(std::stoi(columnText[2]))
            {
                order=std::string("BUY");
            }
            market->addOrder(columnText[0],std::stoi(columnText[1]),order,columnText[4],std::stoi(columnText[3]),std::stoi(columnText[5]),std::stoi(columnText[6]),std::stoi(columnText[7]),std::stoi(columnText[8]));
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT * FROM market");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,this,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}
{
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn==1)
        {
            auto market=static_cast<RessourcesManager*>(data)->m_market;
            market->setOrderSeed(std::stoi(columnText[0]));
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT * FROM order_seed");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,this,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}
}

void RessourcesManager::UpdateOrderSeed(int orderSeed)
{
     char *errorMessage;
    std::string sql=std::string("UPDATE order_seed set order_seed=")+std::to_string(orderSeed);
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}

int RessourcesManager::addOrder(int PCId,std::string side,std::string symbol,int quantity,int price,int stopPrice,bool aon,bool ioc)
{
    return m_market->addOrder(PCId,side,symbol,quantity,price,stopPrice,aon,ioc);
}

bool RessourcesManager::OrderExists(std::string orderID)
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
    std::string sql=std::string("SELECT count(*) FROM market where orderID=")+orderID;
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

void RessourcesManager::on_accept(const orderentry::OrderPtr& order)
{
    if(!OrderExists(order->order_id()))
    {
        char *errorMessage;
        std::string sql=std::string("INSERT INTO market VALUES(")+order->order_id()+std::string(",")+std::to_string(order->PCId())+std::string(",")+std::to_string(order->is_buy())+std::string(",")+std::to_string(order->quantityOnMarket())+std::string(",'")+order->symbol()+std::string("',")+std::to_string(order->price())+std::string(",")+std::to_string(order->stop_price())+std::string(",")+std::to_string(order->all_or_none())+std::string(",")+std::to_string(order->immediate_or_cancel())+std::string(")");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
}

void RessourcesManager::on_fill(const orderentry::OrderPtr& order,const orderentry::OrderPtr& matched_order,liquibook::book::Quantity fill_qty,liquibook::book::Cost fill_cost)
{
    std::function<void(std::string)> func=[](std::string)
    {
    };
    orderentry::OrderPtr orderBuy=order;
    orderentry::OrderPtr orderSell=matched_order;
    if(!orderBuy->is_buy() && orderSell->is_buy())
    {
        std::swap(orderBuy,orderSell);
    }
    AddInventory(func,orderBuy->PCId(),orderBuy->symbol(),fill_qty);
    if(fill_cost-orderBuy->price()*fill_qty>0)
    {
        AddInventory(func,orderBuy->PCId(),"gold",fill_cost-orderBuy->price()*fill_qty);
    }
    AddInventory(func,orderSell->PCId(),"gold",fill_cost);
    char *errorMessage;
    std::string sql;
    if(order->quantityOnMarket()>0)
    {
        sql+=std::string("UPDATE market set quantity=")+std::to_string(order->quantityOnMarket())+std::string(" where orderID=")+order->order_id();
    }
    else
    {
        sql+=std::string("DELETE from market where orderID=")+order->order_id();
    }
    sql+=std::string("; ");
    if(matched_order->quantityOnMarket()>0)
    {
        sql+=std::string("UPDATE market set quantity=")+std::to_string(matched_order->quantityOnMarket())+std::string(" where orderID=")+matched_order->order_id();
    }
    else
    {
        sql+=std::string("DELETE from market where orderID=")+matched_order->order_id();
    }
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
}
