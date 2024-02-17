#pragma once

#include <seastar/http/reply.hh>
#include <seastar/http/handlers.hh>
#include <seastar/core/seastar.hh>
#include "storage/storage.hpp"


class get_keys_handler : public seastar::httpd::handler_base {
public:
    explicit get_keys_handler(seastar::sharded<storage::storage>& storage);

    seastar::future<std::unique_ptr<seastar::http::reply> > handle(const seastar::sstring& path,
            std::unique_ptr<seastar::http::request> req, std::unique_ptr<seastar::http::reply> rep) override;

private:
    seastar::sharded<storage::storage>& _storage;
};