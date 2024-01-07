#pragma once

#include <seastar/core/future.hh>

#include "cache/cache.hpp"

namespace caching{

class cache_service {
public:
    explicit cache_service(int cache_size);
    seastar::future<> run();
    seastar::future<> stop();

    lru_cache& get_cache();
private:
    lru_cache cache;
};

} // namespace caching