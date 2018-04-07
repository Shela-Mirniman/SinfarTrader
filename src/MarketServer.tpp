#include "MarketServer.h"
MarketServer::MarketServer(Pistache::Address addr,size_t thr):m_httpEndpoint(std::make_shared<Pistache::Http::Endpoint>(addr)), m_desc("Sinfar Market API", "0.1")
{
    init(thr);
    start();
}

MarketServer::~MarketServer()
{
    shutdown();
}

void MarketServer::init(size_t thr)
{
    auto opts = Pistache::Http::Endpoint::options()
        .threads(thr)
        .flags(Pistache::Tcp::Options::ReuseAddr);
    m_httpEndpoint->init(opts);
    createDescription();
}

void MarketServer::start()
{
    m_router.initFromDescription(m_desc);

	Pistache::Rest::Swagger swagger(m_desc);
    swagger
        .uiPath("/ui")
        .uiDirectory("/home/pablo/Project/SinfarTrading")
        .apiPath("/api")
        .serializer(&Pistache::Rest::Serializer::rapidJson)
        .install(m_router);
    m_httpEndpoint->setHandler(m_router.handler());
    
    m_httpEndpoint->serveThreaded();
}

void MarketServer::shutdown()
{
    m_httpEndpoint->shutdown();
}

 void MarketServer::createDescription()
 {
        m_desc
            .info()
            .license("Apache", "http://www.apache.org/licenses/LICENSE-2.0");

        auto backendErrorResponse =
            m_desc.response(Pistache::Http::Code::Internal_Server_Error, "An error occured with the backend");

        m_desc
            .schemes(Pistache::Rest::Scheme::Https)
            .basePath("/v1");

        auto versionPath = m_desc.path("/v1");
}
