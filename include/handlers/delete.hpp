#pragma once

#include <seastar/http/reply.hh>
#include <seastar/http/handlers.hh>
#include <seastar/core/seastar.hh>
#include <seastar/core/future.hh>

#include "storage/storage.hpp"


class delete_handler : public seastar::httpd::handler_base {
public:
    delete_handler(seastar::sharded<storage::storage>& storage, const std::string& handler_path);

    seastar::future<std::unique_ptr<seastar::http::reply> > handle(const seastar::sstring& path,
            std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) override;
private:
    seastar::sharded<storage::storage>& _storage;
    std::string handler_path;
};