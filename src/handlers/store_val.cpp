#include <seastar/core/coroutine.hh>

#include "handlers/store_val.hpp"


namespace {

constexpr char delimter[] = ";";

std::optional<std::pair<std::string, std::string>> parse_key_and_val(const std::string& keyAndVal){
    auto pos = keyAndVal.find_first_of(delimter);
    if (pos == std::string::npos) {
        return std::optional<std::pair<std::string, std::string>>();
    }

    std::string key = keyAndVal.substr(0, pos);
    std::string value = keyAndVal.substr(pos+1);
    return std::optional<std::pair<std::string, std::string>>(std::pair(key, value));
}
} // anonymous namespace

store_val_handler::store_val_handler(storage::storage&& storage): _storage(std::move(storage)){}

seastar::future<std::unique_ptr<seastar::http::reply> > store_val_handler::handle(const seastar::sstring& path,
        std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) {
    
    try {
        auto keyAndVal = parse_key_and_val(req->content);
        
        if (!keyAndVal){
            rep->set_status(seastar::http::reply::status_type::bad_request);
            rep->_content = fmt::sprintf("can't parse %s to key and value using %s delimeter", req->content, delimter);
            co_return std::move(rep);
        }

        std::cout << "storing value: " << keyAndVal->second << std::endl;
        std::cout << "for key: " << keyAndVal->first << std::endl;
        
        auto key = keyAndVal->first;
        _storage.store(keyAndVal->first, keyAndVal->second);
        rep->set_status(seastar::http::reply::status_type::created);
        rep->_content = fmt::sprintf("stored key %s", key);
    } catch(...) {
        fmt::print("got error while storing: {}", std::current_exception());
        rep->set_status(seastar::http::reply::status_type::internal_server_error);
    }
    
    co_return std::move(rep);
}