#pragma once

#include <unordered_map>
#include <list>
#include <optional>
#include <string>


namespace caching{

class lru_cache { 
public:
    lru_cache(int max_size);
    void push_front(const std::string& key, const std::string& val);
    void remove(const std::string& key);
    std::optional<std::string> get(const std::string& key) const;
    void display() const;
    int get_size() const;

private:
    struct kv {
        std::string key;
        std::string val;
    };
    std::list<kv> value_list;
    std::unordered_map<std::string, std::list<kv>::iterator> cache;
    int max_size; // maximum capacity of cache
};

} // namespace caching