#include <seastar/core/coroutine.hh>

#include "handlers/get_val.hpp"


get_val_handler::get_val_handler(seastar::sharded<storage::storage>& storage, const std::string& path) : _storage(storage), handler_path(path) {}

seastar::future<std::unique_ptr<seastar::http::reply> > get_val_handler::handle(const seastar::sstring& path,
        std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) {

    try {
        std::string_view key = path;
        key.remove_prefix(std::min(key.find_first_not_of(handler_path+"/"), key.size()));
        auto key_str = std::string{key};

        auto value = co_await _storage.local().get_val(key_str);
        if (!value) {
            rep->set_status(seastar::http::reply::status_type::not_found);
            rep->_content = fmt::sprintf("can't find %s key", key_str);
            co_return std::move(rep);
        }

        co_await _storage.invoke_on_all([key_str, value] (storage::storage& storage) {
            storage.get_cache().push_front(key_str, *value);
        });
        
        rep->set_status(seastar::http::reply::status_type::ok);
        rep->_content = *value;

    } catch(...) {
        fmt::print("got error while getting value: {}", std::current_exception());
        rep->set_status(seastar::http::reply::status_type::internal_server_error);
    }

    co_return std::move(rep);
}