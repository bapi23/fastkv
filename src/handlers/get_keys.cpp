#include <seastar/core/coroutine.hh>

#include "handlers/get_keys.hpp"
#include "cache/cache_service.hpp"

namespace {
    const char delim[] = ";";
}

get_keys_handler::get_keys_handler(storage::storage&& storage) : _storage(std::move(storage)) {}

seastar::future<std::unique_ptr<seastar::http::reply> > get_keys_handler::handle(const seastar::sstring& path,
        std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) {
    try {
        std::vector<std::string> keys = co_await _storage.get_keys();

        std::ostringstream result;
        std::copy(keys.begin(), keys.end(), std::ostream_iterator<std::string>(result, delim));
    
        rep->_content = result.str();
    } catch(...) {
        fmt::print("got error while getting keys: {}", std::current_exception());
        rep->set_status(seastar::http::reply::status_type::internal_server_error);
    }
    co_return std::move(rep);
}