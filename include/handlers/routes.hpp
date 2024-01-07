#pragma once

#include <seastar/http/httpd.hh>

#include "cache/cache_service.hpp"

void set_routes(seastar::httpd::routes& r, seastar::sharded<caching::cache_service>& cache_service);