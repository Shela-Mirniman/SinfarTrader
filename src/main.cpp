#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

#include <pistache/serializer/rapidjson.h>
#include "MarketServer.h"
#include "SinfarClient.h"
#include "Database.h"
#include <signal.h>

sig_atomic_t stopFlag = 0;

void handler( int )
{
    stopFlag = 1;
}

int main(int argc, char *argv[])
{
    signal( SIGTERM, &handler );
    signal( SIGINT, &handler );
    signal( SIGABRT, &handler );
    
    std::shared_ptr<Database> database(std::make_shared<Database>(std::string("testDatabase")));
    
    Pistache::Port port(9080);

    int thr = 4;

    Pistache::Address addr(Pistache::Ipv4::any(), port);

    MarketServer server(addr,thr);
    SinfarClient client(database);
    while(stopFlag == 0)
    {
        client.DoLoop();
    };
}
