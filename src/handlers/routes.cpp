#include "handlers/handlers.hpp"

#include "handlers/routes.hpp"

namespace {
    constexpr char get_path[] = "/get";
    constexpr char get_keys_path[] = "/getkeys";
    constexpr char store_path[] = "/store";
    constexpr char delete_path[] = "/delete";
}

void set_routes(seastar::httpd::routes& r, seastar::sharded<caching::cache_service>& cache_service) {
    namespace httpd = seastar::httpd;
    r.add(httpd::operation_type::GET, httpd::url(get_path).remainder("path"), 
        new get_val_handler{storage::storage{cache_service}, get_path});
    r.add(httpd::operation_type::GET, httpd::url(get_keys_path), 
        new get_keys_handler{storage::storage{cache_service}});
    r.add(httpd::operation_type::POST, httpd::url(store_path).remainder("path"), 
        new store_val_handler{storage::storage{cache_service}});
    r.add(httpd::operation_type::GET, httpd::url(delete_path).remainder("path"), 
        new delete_handler{storage::storage{cache_service}, delete_path});
}