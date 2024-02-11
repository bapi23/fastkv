#include <filesystem>

#include <seastar/core/aligned_buffer.hh>
#include <seastar/core/file.hh>
#include <seastar/core/fstream.hh>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/http/function_handlers.hh>
#include <seastar/http/file_handler.hh>

#include "storage/storage.hpp"

using namespace seastar;
using namespace httpd;

namespace {
    constexpr char root_dir[] = "/tmp/fastkv/";

    constexpr char key_dir[] = "/tmp/fastkv/keys/";
    constexpr char val_dir[] = "/tmp/fastkv/values/";

    constexpr size_t max_value_size = 4096;
    constexpr size_t max_key_size = 512; // looks like that's the smalest buffer which doesn't "invalid argument"
}

namespace storage {

storage::storage(seastar::sharded<caching::cache_service>& cache): cache(cache) {}

future<> storage::store(std::string key, std::string value) {
    auto hash = std::hash<std::string>{}(key);
    sstring keyfilename = key_dir + std::to_string(hash);
    sstring valfilename = val_dir + std::to_string(hash);

    auto wbuf = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(wbuf.get_write(), wbuf.get_write() + max_value_size, 0);
    std::copy(value.begin(), value.end(), wbuf.get_write());
    
    co_await with_file(open_file_dma(valfilename, open_flags::wo | open_flags::create), [&wbuf] (file& f) {
                return f.dma_write(0, wbuf.get(), max_value_size);
            });
    

    auto wbuf_key = temporary_buffer<char>::aligned(max_key_size, max_key_size);
    std::fill(wbuf_key.get_write(), wbuf_key.get_write() + max_key_size, 0);
    std::copy(key.begin(), key.end(), wbuf_key.get_write());


    co_await with_file(open_file_dma(keyfilename, open_flags::wo | open_flags::create), [&wbuf_key] (file& f) {
                return f.dma_write(0, wbuf_key.get(), max_value_size);
            });

    co_await cache.invoke_on_all([key, value](caching::cache_service& cache){
        cache.get_cache().push_front(key, value);
    });

    co_return;
}

future<bool> storage::remove(std::string key){
    auto hash = std::hash<std::string>{}(key);
    sstring keyfilename = key_dir + std::to_string(hash);
    sstring valfilename = val_dir + std::to_string(hash);

    auto exists = co_await seastar::file_exists(keyfilename);

    if (!exists) {
        co_return false;
    }

    co_await seastar::remove_file(keyfilename);
    co_await seastar::remove_file(valfilename);
    co_await cache.invoke_on_all([key] (caching::cache_service& cache) {
                cache.get_cache().remove(key);
        });
    co_return true;
}

future<std::optional<std::string>> storage::get_val(std::string key) const {
    auto value = cache.local().get_cache().get(key);

    if (value) {
        std::cout << "getting value using cache" << std::endl;
        co_await cache.invoke_on_others([key, value] (caching::cache_service& cache) {
            cache.get_cache().push_front(key, *value);
        });

        co_return *value;
    }

    auto hash = std::hash<std::string>{}(key);
    sstring valfilename = val_dir + std::to_string(hash);
    auto exists = co_await seastar::file_exists(valfilename);

    if (!exists) {
        co_return std::nullopt;
    }
    
    auto buffer = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(buffer.get_write(), buffer.get_write() + max_value_size, 0);

    co_await with_file(open_file_dma(valfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_value_size);
    });
    
    sstring content = buffer.get_write(); 
    if (content.empty()) {
        co_return std::nullopt;
    }

    std::cout << "getting value using storage" << std::endl;
    co_await cache.invoke_on_all([key, content] (caching::cache_service& cache) {
        cache.get_cache().push_front(key, content);
    });
    co_return content;
}

future<std::vector<std::string>> storage::get_keys() const {
    std::vector<std::string> hashes;

    namespace fs = std::filesystem;
    for (const auto & entry : fs::directory_iterator(key_dir)) {
        hashes.push_back(entry.path());                            
    }

    std::vector<std::string> keys;
    for (auto keyfilename: hashes) {
        auto buffer = temporary_buffer<char>::aligned(max_key_size, max_key_size);
        std::fill(buffer.get_write(), buffer.get_write() + max_key_size, 0);
        
        co_await with_file(open_file_dma(keyfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_key_size);
        });

        sstring content = buffer.get_write(); 
        keys.push_back(content);
    }

    co_return keys;    
}

} // namespace storage