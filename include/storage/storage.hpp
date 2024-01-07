#pragma once

#include <string>

#include <seastar/core/seastar.hh> 
#include <seastar/core/sharded.hh>


#include "cache/cache_service.hpp"

namespace storage {

class storage {
public:
    storage(seastar::sharded<caching::cache_service>& cache);

    seastar::future<> store(std::string key, std::string value);
    seastar::future<bool> remove(std::string key);
    seastar::future<std::optional<std::string>> get_val(std::string key) const;
    seastar::future<std::vector<std::string>> get_keys() const;

private:
    seastar::sharded<caching::cache_service>& cache;
};

} // namespace storage
