#pragma once

#include <string>

#include <seastar/core/seastar.hh> 
#include <seastar/core/sharded.hh>


#include "cache/cache.hpp"

namespace storage {

class storage {
public:
    storage(const caching::lru_cache& cache);

    seastar::future<> store(std::string key, std::string value);
    seastar::future<bool> remove(std::string key);
    seastar::future<std::optional<std::string>> get_val(std::string key) const;
    seastar::future<std::vector<std::string>> get_keys() const;

    seastar::future<> run();
    seastar::future<> stop();

    caching::lru_cache& get_cache();

private:
    caching::lru_cache cache;
};

} // namespace storage
