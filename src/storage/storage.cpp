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

    constexpr char val_dir[] = "/tmp/fastkv/";

    constexpr size_t max_value_size = 4096;
    constexpr size_t max_key_size = 512; // looks like that's the smalest buffer which doesn't "invalid argument"

    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

}

namespace storage {

storage::storage(int cache_size): cache(cache_size) {
}

future<> storage::store(std::string key, std::string value) {
    auto hash = std::hash<std::string>{}(key);
    sstring valfilename = val_dir + std::to_string(hash);

    auto wbuf = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(wbuf.get_write(), wbuf.get_write() + max_value_size, 0);
    std::fill(wbuf.get_write(), wbuf.get_write() + max_value_size, ' ');
    std::copy(key.begin(), key.end(), wbuf.get_write());
    std::copy(value.begin(), value.end(), wbuf.get_write()+max_key_size);
    

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto bytesWritten = co_await with_file(open_file_dma(valfilename, open_flags::wo | open_flags::create), [&wbuf] (file& f) {
                return f.dma_write(0, wbuf.get(), max_value_size);
            });

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    co_return;
}

future<bool> storage::remove(std::string key){
    auto hash = std::hash<std::string>{}(key);
    sstring valfilename = val_dir + std::to_string(hash);

    auto exists = seastar::file_exists(valfilename).get();

    if (!exists) {
        co_return false;
    }

    seastar::remove_file(valfilename).get();
    co_return true;
}

future<std::optional<std::string>> storage::get_val(std::string key) const {
    auto value = cache.get(key);

    if (value) {
        std::cout << "getting value using cache: " << *value << std::endl;
        co_return *value;
    }
    
    std::cout << "getting value using storage" << std::endl;

    auto hash = std::hash<std::string>{}(key);
    sstring valfilename = val_dir + std::to_string(hash);
    auto exists = co_await seastar::file_exists(valfilename);

    if (!exists) {
        co_return std::nullopt;
    }
    
    auto buffer = temporary_buffer<char>::aligned(max_value_size, max_value_size);
    std::fill(buffer.get_write(), buffer.get_write() + max_value_size, 0);


    auto bytesRead = co_await with_file(open_file_dma(valfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_value_size);
    });
    
    
    std::string content = buffer.get_write();
    std::string val;
    val.resize(content.size() - max_key_size);
    std::copy(content.begin()+max_key_size, content.end(), val.begin());

    if (content.empty()) {
        co_return std::nullopt;
    }

    rtrim(val);
    co_return val;
}

future<std::vector<std::string>> storage::get_keys() const {
    std::vector<std::string> hashes;

    namespace fs = std::filesystem;
    for (const auto & entry : fs::directory_iterator(val_dir)) {
        hashes.push_back(entry.path());                            
    }

    std::vector<std::string> keys;
    for (auto keyfilename: hashes) {
        auto buffer = temporary_buffer<char>::aligned(max_value_size, max_value_size);
        std::fill(buffer.get_write(), buffer.get_write() + max_value_size, 0);
        co_await with_file(open_file_dma(keyfilename, open_flags::ro), [&buffer] (file& f) {
            return f.dma_read(0, buffer.get_write(), max_key_size);
        });

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