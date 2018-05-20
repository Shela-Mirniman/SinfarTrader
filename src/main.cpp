#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

#include <pistache/serializer/rapidjson.h>
#include "MarketServer.h"
#include "SinfarClient.h"
#include "Database.h"
#include <signal.h>
#include "Market.h"
#include "RessourcesManager.h"

sig_atomic_t stopFlag = 0;

void handler( int )
{
    stopFlag = 1;
}

int main(int argc, char *argv[])
{
    try
    {
    while(stopFlag == 0)
    {
    signal( SIGTERM, &handler );
    signal( SIGINT, &handler );
    signal( SIGABRT, &handler );
    std::shared_ptr<Database> database(std::make_shared<Database>(std::string("testDatabase")));
    std::shared_ptr<RessourcesManager> ressource(std::make_shared<RessourcesManager>(database));
    std::shared_ptr<orderentry::Market> market(std::make_shared<orderentry::Market>(ressource));
    ressource->AddMarket(market);
    
    Pistache::Port port(9080);

    int thr = 4;

    Pistache::Address addr(Pistache::Ipv4::any(), port);

    MarketServer server(addr,thr);
    std::shared_ptr<SinfarClient> client(std::make_shared<SinfarClient>(ressource));
    ressource->AddClient(client);
    
    ressource->UpdateMarket();
    while(stopFlag == 0)
    {
            client->DoLoop();
    };
    }
    }
    catch(const std::exception& ex)
    {
        std::cout<<"exception catch in outer loop "<<ex.what()<<std::endl;
    }
    catch (const std::string& ex)
    {
        std::cout<<"exception catch in outer loop "<<ex<<std::endl;
    }
    catch (...)
    {
        std::cout<<"exception catch in outer loop"<<std::endl;
    }
}
