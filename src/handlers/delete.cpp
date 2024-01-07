#include <seastar/core/coroutine.hh>

#include "handlers/delete.hpp"


delete_handler::delete_handler(
    storage::storage&& storage, const std::string& path): _storage(std::move(storage)), handler_path(path){}

seastar::future<std::unique_ptr<seastar::http::reply> > delete_handler::handle(const seastar::sstring& path,
        std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) {
    
    try {
        std::string_view key = path;
        key.remove_prefix(std::min(key.find_first_not_of(handler_path+"/"), key.size()));

        bool deleted = co_await _storage.remove(std::string{key});
        if (deleted) {
            rep->set_status(seastar::http::reply::status_type::ok);
        } else {
            rep->_content = fmt::sprintf("key %s doesn't exist", key);
            rep->set_status(seastar::http::reply::status_type::not_found);
        }
    } catch(...) {
        fmt::print("got error while deleting: {}", std::current_exception());
        rep->set_status(seastar::http::reply::status_type::internal_server_error);
    }

    co_return std::move(rep);
}