#pragma once

#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

#include <pistache/net.h>
#include <pistache/client.h>

#include <pistache/serializer/rapidjson.h>

class MarketServer
{
    std::shared_ptr<Pistache::Http::Endpoint> m_httpEndpoint;
    Pistache::Rest::Description m_desc;
    Pistache::Rest::Router m_router;
    public:
        MarketServer(Pistache::Address addr,size_t thr);
        ~MarketServer();
    private:
        void init(size_t thr);
        void start();
        void shutdown();
        void createDescription();
};

#include "MarketServer.tpp"
