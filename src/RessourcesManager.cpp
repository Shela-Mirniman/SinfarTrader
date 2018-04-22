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

void RessourcesManager::AddClient(std::shared_ptr<SinfarClient> client)
{
    m_client=client;
}

void RessourcesManager::ListInventory(std::function<void(std::string)> func,int PCId)
{
    std::string message("Lists of Goods in Inventory:\n");
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
            if(Quantity>=0)
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
            std::string sql=std::string("UPDATE goodsList set goodsDescription='")+Escape(GoodsDescription)+std::string("' where goodsName='")+Goodsname+std::string("'");
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        else
        {
            char *errorMessage;
            std::string sql=std::string("INSERT INTO goodsList VALUES('")+Goodsname+std::string("','")+Escape(GoodsDescription)+std::string("')");
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
            if(!m_market->symbolIsDefined(Goodsname))
            {
                m_market->addBook(Goodsname);
            }
        }
        func(std::string("Goods added sucessfuly: Goodsname=")+Goodsname+std::string(" GoodsDescription=")+Escape(GoodsDescription));
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

void RessourcesManager::AddAccount(std::function<void(std::string)> func,int PCId,bool Employee,int PlayerId,std::string name,std::string PlayerName,int fee)
{
        int permission_level=0;
        if(Employee)
        {
            permission_level=5;
        }
        if(HasAccount(PCId))
        {
            char *errorMessage;
            std::string sql=std::string("UPDATE account set PlayerId=")+std::to_string(PlayerId)+std::string(" , PlayerName='")+PlayerName+std::string("' , name='")+Escape(name)+std::string("' , permission_level=")+std::to_string(permission_level)+(", fee=")+std::to_string(fee)+std::string(" where PCId=")+std::to_string(PCId);
            std::cout<<"sql "<<sql<<std::endl;
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        else
        {
            char *errorMessage;
            std::string sql=std::string("INSERT INTO account VALUES(")+std::to_string(PCId)+std::string(",")+std::to_string(PlayerId)+std::string(",'")+PlayerName+std::string("','")+Escape(name)+std::string("',")       +std::to_string(permission_level)+std::string(",")+std::to_string(fee)+std::string(")");
            std::cout<<"sql "<<sql<<std::endl;
            if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
            {
                throw std::runtime_error(errorMessage);
            }
        }
        func(std::string("Add sucessful: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+Escape(name)+std::string(" PlayerName=")+PlayerName+std::string(" Permission_Level=")+std::to_string(permission_level)+std::string(" Fee=")+std::to_string(fee));
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
            int fee=static_cast<RessourcesManager*>(data)->GetFee(std::stoi(columnText[1]));
            market->addOrder(columnText[0],std::stoi(columnText[1]),order,columnText[4],std::stoi(columnText[3]),std::stoi(columnText[5]),fee,std::stoi(columnText[6]),std::stoi(columnText[7]),std::stoi(columnText[8]));
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

int RessourcesManager::addOrder(int PCId,std::string side,std::string symbol,int quantity,int price,int fee,int stopPrice,bool aon,bool ioc)
{
    return m_market->addOrder(PCId,side,symbol,quantity,price,fee,stopPrice,aon,ioc);
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
    auto func=m_client->GetMessager(order->PCId());
    try
    {
    if(!OrderExists(order->order_id()))
    {
        char *errorMessage;
        std::string sql=std::string("INSERT INTO market VALUES(")+order->order_id()+std::string(",")+std::to_string(order->PCId())+std::string(",")+std::to_string(order->is_buy())+std::string(",")+std::to_string(order->quantityOnMarket())+std::string(",'")+order->symbol()+std::string("',")+std::to_string(order->price())+std::string(",")+std::to_string(order->stop_price())+std::string(",")+std::to_string(order->all_or_none())+std::string(",")+std::to_string(order->immediate_or_cancel())+std::string(")");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        std::string type_Order("Sell");
        if(order->is_buy())
        {
            type_Order=std::string("Buy");
        }
        if(order->is_buy())
        {
            int fee=order->fee();
            RemoveInventory(func,order->PCId(),std::string("gold"),order->quantityOnMarket()*order->price()*(1+fee*0.01));
        }
        else
        {
            RemoveInventory(func,order->PCId(),order->symbol(),order->quantityOnMarket());
        }
        func(type_Order+std::string(" order accepted for ")+std::to_string(order->quantityOnMarket())+std::string(" ")+order->symbol()+std::string(" at ")+std::to_string(order->price()));
    }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::on_fill(const orderentry::OrderPtr& order,const orderentry::OrderPtr& matched_order,liquibook::book::Quantity fill_qty,liquibook::book::Cost fill_cost)
{
    auto func=m_client->GetMessager(order->PCId());
    auto func2=m_client->GetMessager(matched_order->PCId());
    try
    {
    orderentry::OrderPtr orderBuy=order;
    orderentry::OrderPtr orderSell=matched_order;
    if(!orderBuy->is_buy() && orderSell->is_buy())
    {
        std::swap(orderBuy,orderSell);
    }
    auto funcBuy=m_client->GetMessager(orderBuy->PCId());
    auto funcSell=m_client->GetMessager(orderSell->PCId());
    int BuyFee=GetFee(orderBuy->PCId());
    int SellFee=GetFee(orderSell->PCId());;
    funcBuy(std::string("Buy Order filled ")+std::to_string(fill_qty)+std::string(" ")+orderBuy->symbol()+std::string(" for ")+std::to_string(fill_cost)+std::string(" total fee=")+std::to_string(int((BuyFee*0.01)*fill_cost)));
    funcSell(std::string("Sell Order filled ")+std::to_string(fill_qty)+std::string(" ")+orderSell->symbol()+std::string(" for ")+std::to_string(fill_cost)+std::string(" total fee=")+std::to_string(int((SellFee*0.01)*fill_cost)));
    AddFilled(orderBuy->PCId(),orderBuy->symbol(),true,fill_qty,fill_cost,funcBuy);
    AddFilled(orderSell->PCId(),orderSell->symbol(),false,fill_qty,fill_cost,funcSell);
    AddInventory(funcBuy,orderBuy->PCId(),orderBuy->symbol(),fill_qty);
    if(orderBuy->price()*fill_qty-fill_cost>0)
    {
        AddInventory(funcBuy,orderBuy->PCId(),"gold",(orderBuy->price()*fill_qty-fill_cost)*(1+BuyFee*0.01));
    }
    AddInventory(funcSell,orderSell->PCId(),"gold",fill_cost*(1-SellFee*0.01));
    std::function<void(std::string)> funcempty=[](std::string message){};
    shopAdd((BuyFee+SellFee)*0.01*fill_cost,funcempty);
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
    catch(const std::exception& ex)
    {
        func(ex.what());
        func2(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
        func2(ex);
    }
}

void RessourcesManager::DebugListMarket(std::function<void(std::string)> func)
{
        std::string message("Lists of Orders:\n");
        auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==9)
            {
                *static_cast<std::string*>(data)+=std::string("OrderID=")+std::string(columnText[0])+std::string(" PCId=")+std::string(columnText[1])+std::string(" IsBuy=")+std::string(columnText[2])+std::string(" GoodsName=")+std::string(columnText[4])+std::string(" Quantity=")+std::string(columnText[3])+std::string(" Price=")+std::string(columnText[5])+std::string("\n");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT * FROM market");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        func(message);
}

void RessourcesManager::ListOrderMarket(std::function<void(std::string)> func,int PCId)
{
    std::string message("Lists of MyOrders:\n");
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==9)
            {
                std::string action(" SELL ");
                if(std::stoi(columnText[2]))
                {
                    action=" BUY ";
                }
                *static_cast<std::string*>(data)+=std::string("OrderID=")+std::string(columnText[0])+action+std::string(" GoodsName=")+std::string(columnText[4])+std::string(" Quantity=")+std::string(columnText[3])+std::string(" Price=")+std::string(columnText[5])+std::string("\n");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT * FROM market where PCId="+std::to_string(PCId));
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        func(message);
}

void RessourcesManager::ListPriceBook(std::function<void(std::string)> func,std::string GoodsName)
{
    m_market->ListPriceBook(func,GoodsName);
}

void RessourcesManager::OrderInfo(std::string orderID,int& price,int& quantity,std::string& goodsName,bool& isBuy)
{
    auto order=m_market->GetOrder(orderID);
    goodsName=order->symbol();
    price=order->price();
    quantity=order->quantityOnMarket()+order->quantityFilled();
    isBuy=order->is_buy();
}

bool RessourcesManager::replaceOrder(int PCId,int orderID,int dquantity,int newprice,std::function<void(std::string)> func)
{
    auto order=m_market->GetOrder(std::to_string(orderID));
    if(order->PCId()==PCId)
    {
        std::string goodsName=order->symbol();
        auto book=m_market->findBook(goodsName);
        bool sucess=book->replace(order,dquantity,newprice);
        return sucess;
    }
    else
    {
        func("Cannot replace an order which is not yours.");
    }
}

void RessourcesManager::Command_InventoryAdd(int PCId,int PCIdTo,std::string GoodsName,int Quantity,std::function<void(std::string)> func)
{
    if(Quantity>0&&PCIdTo>0&&IsEmployee(PCId))
    {
        if(HasGoods(GoodsName))
        {
            try
            {
                AddInventory(func,PCIdTo,GoodsName,Quantity);
            }
            catch(const std::exception& ex)
            {
                func(ex.what());
            }
            catch (const std::string& ex)
            {
                func(ex);
            }
        }
        else
        {
            func(std::string("GoodsName non existant: ")+GoodsName);
        }
    }
}

void RessourcesManager::Command_InventoryRemove(int PCId,int PCIdTo,std::string GoodsName,int Quantity,std::function<void(std::string)> func)
{
    if(Quantity>0&&PCIdTo>0 && IsEmployee(PCId))
    {
        if(HasGoods(GoodsName))
        {
            try
            {
                RemoveInventory(func,PCIdTo,GoodsName,Quantity);
            }
            catch(const std::exception& ex)
            {
                func(ex.what());
            }
            catch (const std::string& ex)
            {
                func(ex);
            }
        }
        else
        {
            func(std::string("GoodsName non existant: ")+GoodsName);
        }
    }
}

void RessourcesManager::Command_ListInventory(int PCId,int PCIdTo,std::function<void(std::string)> func)
{
    if(PCIdTo==-1)
    {
        PCIdTo=PCId;
    }
    else if(IsEmployee(PCId))
    {
        return;
    }
    if(PCIdTo>0)
    {
        try
        {
            ListInventory(func,PCIdTo);
        }
        catch(const std::exception& ex)
        {
            func(ex.what());
        }
        catch (const std::string& ex)
        {
            func(ex);
        }
    }
}

void RessourcesManager::Command_TradeBuy(int PCId,std::string GoodsName,int Quantity,int Price,std::function<void(std::string)> func)
{
    if(Quantity>0&&Price>0)
    {
        try
        {
        if(HasGoods(GoodsName)&& GoodsName!=std::string("gold"))
        {
            int goldAvailable=GetInventory(PCId,"gold");
            int fee=GetFee(PCId);
            if(goldAvailable>=Price*Quantity*(1.0+fee*0.01))
            {
                int idOrder=addOrder(PCId,"BUY",GoodsName,Quantity,Price,fee);
            }
            else if(GoodsName==std::string("gold"))
            {
                func(std::string("You cannot trade gold with gold!!!!:"));
            }
            else
            {
                func(std::string("You do not have enought gold: ")+GoodsName);
            }
        }
        else
        {
            func(std::string("GoodsName non existant: ")+GoodsName);
        }
        }
        catch(const std::exception& ex)
        {
            func(ex.what());
        }
        catch (const std::string& ex)
        {
            func(ex);
        }
    }
}

void RessourcesManager::Command_TradeSell(int PCId,std::string GoodsName,int Quantity,int Price,std::function<void(std::string)> func)
{
    if(Quantity>0&&Price>0)
    {
        try
        {
            if(HasGoods(GoodsName)&&GoodsName!=std::string("gold"))
            {
                int goodsAvailable=GetInventory(PCId,GoodsName);
                int fee=GetFee(PCId);
                if(goodsAvailable>=Quantity)
                {
                    int idOrder=addOrder(PCId,"SELL",GoodsName,Quantity,Price,fee);
                }
                else
                {
                    func(std::string("You do not have enought Goods: ")+GoodsName);
                }
            }
            else if(GoodsName==std::string("gold"))
            {
                func(std::string("You cannot trade gold with gold!!!!:"));
            }
            else
            {
                func(std::string("GoodsName non existant: ")+GoodsName);
            }
        }
        catch(const std::exception& ex)
        {
            func(ex.what());
        }
        catch (const std::string& ex)
        {
            func(ex);
        }
    }
}

void RessourcesManager::Command_Replace(int PCId,int orderID,int dQuantity,int Price,std::function<void(std::string)> func)
{
    if(Price>0)
    {
        try
        {
            int priceOld;
            int quantityOld;
            std::string goodsName;
            bool isBuy;
            OrderInfo(std::to_string(orderID),priceOld,quantityOld,goodsName,isBuy);
            if(isBuy&&HasGoods(goodsName)&& goodsName!=std::string("gold"))
            {
                int goldAvailable=GetInventory(PCId,"gold");
                if(goldAvailable>=(Price*(quantityOld+dQuantity)-priceOld*quantityOld))
                {
                    int priceChange;
                    int quantityChange;
                    int totalPriceChange;
                    bool sucess=replaceOrder(PCId,orderID,dQuantity,Price,func);
                }
            }
            else if(!isBuy&&HasGoods(goodsName)&& goodsName!=std::string("gold"))
            {
                int goodsAvailable=GetInventory(PCId,goodsName);
                if(goodsAvailable>=dQuantity)
                {
                    int priceChange;
                    int quantityChange;
                    int totalPriceChange;
                    bool sucess=replaceOrder(PCId,orderID,dQuantity,Price,func);
                }
            }
            else if(goodsName==std::string("gold"))
            {
                func(std::string("You cannot trade gold with gold!!!!:"));
            }
        }
        catch(const std::exception& ex)
        {
            func(ex.what());
        }
        catch (const std::string& ex)
        {
            func(ex);
        }
    }
}

void RessourcesManager::Command_Cancel(int PCId,int orderID, std::function<void(std::string)> func)
{
    auto order=m_market->GetOrder(std::to_string(orderID));
    if(order->PCId()==PCId)
    {
        std::string goodsName=order->symbol();
        auto book=m_market->findBook(goodsName);
        book->cancel(order);
    }
    else
    {
        func("You cannot cancel an order which is not yours");
    }
}

void RessourcesManager::Command_TradeListPrice(std::string GoodsName,std::function<void(std::string)> func)
{
    try
    {
        if(HasGoods(GoodsName))
        {
            ListPriceBook(func,GoodsName);
        }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::Command_TradeHistory(int PCId,std::function<void(std::string)> func)
{
    try
    {
        std::string message("Trading History:\n");
        auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==6)
            {
                std::string type_order("Sell");
                if(std::stoi(columnText[2]))
                {
                    type_order=std::string("Buy");
                }
                *static_cast<std::string*>(data)+=type_order + std::string(" ")+std::string(columnText[1])+std::string(" ")+std::string(columnText[3])+std::string(" at ")+std::string(columnText[4])+std::string(" gp ")+std::string(columnText[5])+std::string(" \n");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT * FROM filled_history where PCId=")+std::to_string(PCId)+std::string(" Order By date DESC LIMIT 10");
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        func(message);
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::Command_AddAccount(std::string PlayerName,int PCId,int PCIdToAdd,bool EmployeeToAdd,int fee,std::function<void(std::string)> func)
{
    if(IsEmployee(PCId))
    {
        if(EmployeeToAdd)
        {
            if(!IsAdmin(PCId))
            {
                func("Need to be admin to add an employee.");
            }
        }
        m_client->AddAccount(PlayerName,PCIdToAdd,fee,EmployeeToAdd);
    }
}

void RessourcesManager::Command_NewGoods(int PCId,std::string GoodsName,std::string GoodsDescription,std::function<void(std::string)> func)
{
    if(IsEmployee(PCId))
    {
        try
        {
            AddGoods(func,GoodsName,GoodsDescription);
        }
        catch(const std::exception& ex)
        {
            func(ex.what());
        }
        catch (const std::string& ex)
        {
            func(ex);
        }
    }
}

void RessourcesManager::Command_DeleteAccount(int PCId,int PCIdToDelete, std::function<void(std::string)> func)
{
    if(IsAdmin(PCId))
    {
        DeleteAccount(PCIdToDelete,func);
    }
}

void RessourcesManager::DeleteAccount(int PCId,std::function<void(std::string)> func)
{
    try
    {
        int PlayerId;
        std::string name;
        std::string PlayerName;
        int permission_level=0;
        m_client->GetPCInformation(PCId,PlayerId,name,PlayerName);
        if(HasAccount(PCId))
        {
            DeleteAccount(PCId);
        }
        else
        {
            func(std::string("No Account found: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName);
        }
        func(std::string("Delete sucessful: PCId=")+std::to_string(PCId)+std::string(" PlayerID=")+std::to_string(PlayerId)+std::string(" PCName=")+name+std::string(" PlayerName=")+PlayerName);
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::on_reject(const orderentry::OrderPtr& order, const char* reason)
{
    int PCId=order->PCId();
    auto func=m_client->GetMessager(PCId);
    func(std::string("Order rejected: ")+std::string(reason));
}

void RessourcesManager::on_cancel(const orderentry::OrderPtr& order)
{
    int PCId=order->PCId();
    auto func=m_client->GetMessager(PCId);
    try
    {
    if(order->is_buy())
    {
        int fee=GetFee(PCId);
        AddInventory(func,PCId,"gold",order->price()*order->order_qty()*(1+fee*0.01));
    }
    else
    {
        AddInventory(func,PCId,order->symbol(),order->order_qty());
    }
    
    char *errorMessage;
    std::string sql=std::string("DELETE from market where orderID=")+order->order_id();
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
    func(std::string("Order canceled."));
}

void RessourcesManager::on_cancel_reject(const orderentry::OrderPtr& order, const char* reason)
{
    int PCId=order->PCId();
    auto func=m_client->GetMessager(PCId);
    func(std::string("Cancelling rejected: ")+std::string(reason));
}

void RessourcesManager::on_replace(const orderentry::OrderPtr& order,const int32_t& size_delta,liquibook::book::Price new_price)
{
    int PCId=order->PCId();
    auto func=m_client->GetMessager(PCId);
    try
    {
    char *errorMessage;
    std::string sql=std::string("UPDATE market set quantity=")+std::to_string(order->quantityOnMarket())+std::string(", price=")+std::to_string(new_price)+ std::string(" where orderID=")+order->order_id();
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    if(order->is_buy())
    {
        int fee=GetFee(PCId);
        int dgold=(order->quantityOnMarket()-size_delta)*order->price()*(1+fee*0.01)-(order->quantityOnMarket())*new_price*(1+fee*0.01);
        if(dgold>0)
        {
            AddInventory(func,PCId,"gold",dgold);
        }
        else if(dgold<0)
        {
            RemoveInventory(func,PCId,"gold",-dgold);
        }
    }
    else
    {
        if(size_delta>0)
        {
            RemoveInventory(func,PCId,order->symbol(),size_delta);
        }
        else if(size_delta<0)
        {
            AddInventory(func,PCId,order->symbol(),-size_delta);
        }
    }
    func(std::string("Order replaced."));
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
    
}

void RessourcesManager::on_replace_reject(const orderentry::OrderPtr& order, const char* reason)
{
    int PCId=order->PCId();
    auto func=m_client->GetMessager(PCId);
    func(std::string("Replacement rejected: ")+std::string(reason));
}

void RessourcesManager::AddFilled(int PCId,std::string GoodsName,bool isBuy,int Quantity,int FilledPrice,std::function<void(std::string)> func)
{
    try
    {
    char *errorMessage;
    std::string sql=std::string("INSERT INTO filled_history VALUES(")+std::to_string(PCId)+std::string(",'")+GoodsName+std::string("',")+std::to_string(isBuy)+std::string(",")+std::to_string(Quantity)+std::string(",")+std::to_string(FilledPrice)+std::string(",datetime('now'))");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::Command_Info(int PCId,std::function<void(std::string)> func)
{
    try
    {
        std::string message("fee=");
        auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
        {
            if(nbcolumn==1)
            {
                *static_cast<std::string*>(data)+=std::string(columnText[0])+std::string("%");
            }
            return 0;
        };
        char *errorMessage;
        std::string sql=std::string("SELECT fee FROM account where PCId=")+std::to_string(PCId);
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&message,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
        func(message);
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::Command_shopAdd(int PCId,int quantity,std::function<void(std::string)> func)
{
    try
    {
        if(IsAdmin(PCId))
        {
            shopAdd(quantity,func);
        }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}
void RessourcesManager::Command_shopRemove(int PCId,int quantity,std::function<void(std::string)> func)
{
    try
    {
        if(IsAdmin(PCId))
        {
            shopRemove(quantity,func);
        }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}
void RessourcesManager::Command_shopInfo(int PCId,std::function<void(std::string)> func)
{
    try
    {
        if(IsAdmin(PCId))
        {
            shopInfo(func);
        }
    }
    catch(const std::exception& ex)
    {
        func(ex.what());
    }
    catch (const std::string& ex)
    {
        func(ex);
    }
}

void RessourcesManager::shopAdd(int quantity,std::function<void(std::string)> func)
{
    int gold=shopGet(func)+quantity;
    if(gold>=0)
    {
        char *errorMessage;
        std::string sql=std::string("UPDATE shop_gold set shop_gold=")+std::to_string(gold);
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
    else
    {
        throw std::runtime_error("Gold becoming negatif");
    }
}
void RessourcesManager::shopRemove(int quantity,std::function<void(std::string)> func)
{
    int gold=shopGet(func)-quantity;
    if(gold>=0)
    {
        char *errorMessage;
        std::string sql=std::string("UPDATE shop_gold set shop_gold=")+std::to_string(gold);
        if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),nullptr,nullptr,&errorMessage)!=SQLITE_OK)
        {
            throw std::runtime_error(errorMessage);
        }
    }
    else
    {
        throw std::runtime_error("Gold becoming negatif");
    }
}
void RessourcesManager::shopInfo(std::function<void(std::string)> func)
{
    int gold=shopGet(func);
    func(std::string("Shop gold=")+std::to_string(gold)+std::string("gp"));
}
int RessourcesManager::shopGet(std::function<void(std::string)> func)
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
    std::string sql=std::string("SELECT shop_gold FROM shop_gold");
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&banswer,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return banswer;
}

int RessourcesManager::GetFee(int PCId)
{
    int fee=0;
    auto callback=[](void* data,int nbcolumn,char ** columnText,char ** columnName)-> int
    {
        if(nbcolumn==1)
        {
            *static_cast<int*>(data)=std::stoi(columnText[0]);
        }
        return 0;
    };
    char *errorMessage;
    std::string sql=std::string("SELECT fee FROM account where PCId=")+std::to_string(PCId);
    if(sqlite3_exec(m_database->m_sqlite,sql.c_str(),callback,&fee,&errorMessage)!=SQLITE_OK)
    {
        throw std::runtime_error(errorMessage);
    }
    return fee;
}
