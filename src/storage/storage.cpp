#include <filesystem>

#include <seastar/core/aligned_buffer.hh>
#include <seastar/core/file.hh>
#include <seastar/core/fstream.hh>
#include <seastar/core/temporary_buffer.hh>
#include <seastar/http/function_handlers.hh>
#include <seastar/http/file_handler.hh>

#include <seastar/core/reactor.hh>

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

    storage::storage(const caching::lru_cache& cache): cache(cache) {
    }

future<> storage::store(std::string key, std::string value) {

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    auto hash = std::hash<std::string>{}(key);
    sstring keyfilename = key_dir + std::to_string(hash);
    sstring valfilename = val_dir + std::to_string(hash);

    auto wbuf = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(wbuf.get_write(), wbuf.get_write() + max_value_size, 0);
    std::copy(value.begin(), value.end(), wbuf.get_write());
    
    std::cout << "thread ID1: " << std::this_thread::get_id()  << std::endl;

    with_file(open_file_dma(valfilename, open_flags::wo | open_flags::create), [&wbuf] (file& f) {
                return f.dma_write(0, wbuf.get(), max_value_size);
            }).get();
    

    auto wbuf_key = temporary_buffer<char>::aligned(max_key_size, max_key_size);
    std::fill(wbuf_key.get_write(), wbuf_key.get_write() + max_key_size, 0);
    std::copy(key.begin(), key.end(), wbuf_key.get_write());

    std::cout << "thread ID2: " << std::this_thread::get_id() << std::endl;

    with_file(open_file_dma(keyfilename, open_flags::wo | open_flags::create), [&wbuf_key] (file& f) {
                return f.dma_write(0, wbuf_key.get(), max_value_size);
            }).get();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time difference = " 
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() 
              << "[ms]" << std::endl;
    std::cout << "thread ID3: " << std::this_thread::get_id()  << std::endl;

    co_return;
}

future<bool> storage::remove(std::string key){
    auto hash = std::hash<std::string>{}(key);
    sstring keyfilename = key_dir + std::to_string(hash);
    sstring valfilename = val_dir + std::to_string(hash);

    auto exists = seastar::file_exists(keyfilename).get();

    if (!exists) {
        co_return false;
    }

    seastar::remove_file(keyfilename).get();
    seastar::remove_file(valfilename).get();
    co_return true;
}

future<std::optional<std::string>> storage::get_val(std::string key) const {
    auto value = cache.get(key);

    if (value) {
        std::cout << "getting value using cache" << std::endl;
        co_return *value;
    }

    auto hash = std::hash<std::string>{}(key);
    sstring valfilename = val_dir + std::to_string(hash);
    auto exists = seastar::file_exists(valfilename).get();

    if (!exists) {
        co_return std::nullopt;
    }
    
    auto buffer = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(buffer.get_write(), buffer.get_write() + max_value_size, 0);

    with_file(open_file_dma(valfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_value_size);
    }).get();
    
    sstring content = buffer.get_write(); 
    if (content.empty()) {
        co_return std::nullopt;
    }

    std::cout << "getting value using storage" << std::endl;
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
        
        with_file(open_file_dma(keyfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_key_size);
        }).get();

        sstring content = buffer.get_write(); 
        keys.push_back(content);
    }

    co_return keys;    
}

seastar::future<> storage::run() {
    std::cerr << "running storage service on thred number: " << seastar::engine().cpu_id() << "\n";
    return seastar::make_ready_future<>();
}
seastar::future<> storage::stop() {
    return seastar::make_ready_future<>();
}

caching::lru_cache& storage::get_cache() {
    return cache;
}

} // namespace storage