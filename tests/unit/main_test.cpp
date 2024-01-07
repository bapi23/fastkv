#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>

#include "cache/cache.hpp"


TEST_CASE("value pushed should provide same value", "lru_tests") {
    caching::lru_cache cache(10);

    cache.push_front("1", "data");
    auto val = cache.get("1");
    REQUIRE(val == "data");
}

TEST_CASE("key removed should provide nullopt", "lru_tests") {
    caching::lru_cache cache(10);

    cache.push_front("1", "data");
    cache.remove("1");
    auto val = cache.get("1");
    REQUIRE(val == std::nullopt);
}

TEST_CASE("more values than size_max provided should not return old one", "lru_tests") {
    caching::lru_cache cache(5);

    cache.push_front("1", "data1");
    cache.push_front("2", "data2");
    cache.push_front("3", "data3");
    cache.push_front("4", "data4");
    cache.push_front("5", "data5");
    cache.push_front("6", "data6");
    cache.display();
    auto val = cache.get("1");
    REQUIRE_FALSE(val);
}

TEST_CASE("max size reached but key is already in cache - should preserve all keys", "lru_tests") {
    const int max_size = 2;
    caching::lru_cache cache(max_size);

    cache.push_front("1", "data1");
    cache.push_front("2", "data5");
    cache.push_front("2", "data8");
    cache.display();
    auto val = cache.get("2");
    REQUIRE(*val == "data8");
    auto val2 = cache.get("1");
    REQUIRE(*val2 == "data1");
    REQUIRE(cache.get_size() == max_size);
}