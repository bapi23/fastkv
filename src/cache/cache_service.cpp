#include "cache/cache_service.hpp"

#include "seastar/core/reactor.hh"

namespace caching {

cache_service::cache_service(int cache_size): cache(cache_size){}

seastar::future<> cache_service::run() {
    std::cerr << "running cache service on thred number: " << seastar::engine().cpu_id() << "\n";
    return seastar::make_ready_future<>();
}
seastar::future<> cache_service::stop() {
    return seastar::make_ready_future<>();
}

lru_cache& cache_service::get_cache() {
    return cache;
}

} // namespace caching