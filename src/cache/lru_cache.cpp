#include "cache/cache.hpp"
#include <iostream>
#include <optional>

namespace caching {

lru_cache::lru_cache(int max_size): max_size(max_size) {}

std::optional<std::string> lru_cache::get(const std::string& key) const {
    auto it = cache.find(key);

    if (it == cache.end()){
        return std::nullopt;
    }
    return it->second->val;
}

void lru_cache::remove(const std::string& key) {
    auto it = cache.find(key);

    if (it == cache.end()){
        return;
    }

    value_list.erase(it->second);
    cache.erase(it);
}

void lru_cache::push_front(const std::string& key, const std::string& val) {    
    auto it = cache.find(key);

    if (it != cache.end()) {
        value_list.erase(cache[key]);
    } else {
        if (value_list.size() == max_size) {
            auto last = value_list.back();
            value_list.pop_back();

            cache.erase(last.key);
        }
    }

    value_list.push_front(kv{.key = key, .val = val});
    cache[key] = value_list.begin();
}
 
void lru_cache::display() const {
    for(const auto& it: value_list)
        std::cout << it.key << ", " << it.val << " ";
    
    std::cout << std::endl;
   for (auto& it: cache) {
        std::cout << it.first << " ";
    }
    std::cout << std::endl;
}

int lru_cache::get_size() const {
    return max_size;
}

} // namespace caching