#pragma once

#include <seastar/http/httpd.hh>

#include "storage/storage.hpp"

void set_routes(seastar::httpd::routes& r, seastar::sharded<storage::storage>& cache_service);