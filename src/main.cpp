#include <seastar/http/httpd.hh>
#include <seastar/core/seastar.hh>
#include <seastar/net/inet_address.hh>
#include <seastar/util/defer.hh>
#include <seastar/util/closeable.hh>
#include <seastar/core/app-template.hh>
#include <seastar/http/api_docs.hh>
#include <seastar/core/thread.hh>

#include "stop_signal.hh"
#include "handlers/handlers.hpp"
#include "handlers/routes.hpp"


namespace bpo = boost::program_options;

int main(int ac, char** av) {
    seastar::httpd::http_server_control server_control;
    seastar::app_template app;

    app.add_options()
        ("cache-size,cs", bpo::value<int>()->default_value(100), "size of the lru cache in number of entries")
        ("port,p", bpo::value<uint16_t>()->default_value(8899), "port number to start storage service")
        ;

    return app.run(ac, av, [&app] {
        auto& args = app.configuration();
        auto cache_size = args["cache-size"].as<int>();
        auto port = args["port"].as<uint16_t>();
        std::cout << "configured cache size: " << args["cache-size"].as<int>() << "\n";
        
        return seastar::async([cache_size, port] {
            seastar_apps_lib::stop_signal stop_signal;

            seastar::sharded<storage::storage> service;
            service.start(cache_size).then([&service] (){
                return service.invoke_on_all([] (storage::storage& service) {
                    return service.run();
                });
            });
            auto stop_storage_service = seastar::deferred_stop(service);

            auto server = new seastar::httpd::http_server_control();
            server->start().get();

            auto stop_server = seastar::defer([&] () noexcept {
                std::cout << "Stoppping HTTP server" << std::endl;
                server->stop().get();
            });

            server->set_routes([&service](seastar::httpd::routes& r){set_routes(r, service);}).get();
            
            std::cout << "Seastar HTTP server listening on port " << port << " ...\n";
            server->listen(port).get();

            stop_signal.wait().get();
            return 0;
        });
    });
}