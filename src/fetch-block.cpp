#include <bitcoin/bitcoin.hpp>
#include <obelisk/obelisk.hpp>
#include "config.hpp"
#include "util.hpp"

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;

bool stopped = false;

void block_fetched(const std::error_code& ec,
    const hash_digest_list& blk)
{
    if (ec)
    {
        std::cerr << "fetch-block: " << ec.message() << std::endl;
        stopped = true;
        return;
    }
    data_chunk raw_blk(satoshi_raw_size(blk));
    satoshi_save(blk, raw_blk.begin());
    std::cout << raw_blk << std::endl;
    stopped = true;
}

int main(int argc, char** argv)
{
    std::string index_str;
    size_t height = 0;
    if (argc == 2)
        index_str = argv[1];
    else
        index_str = read_stdin();
    config_map_type config;
    load_config(config);
    threadpool pool(1);
    obelisk::fullnode_interface fullnode(pool, config["service"],
        config["client-certificate"], config["server-public-key"]);

    try
    {
        height = boost::lexical_cast<size_t>(index_str);
    }
    catch (const boost::bad_lexical_cast&)
    {
        std::cerr << "fetch-block: Bad index provided." << std::endl;
        return -1;
    }
    fullnode.blockchain.fetch_block_header(height, block_fetched);
    while (!stopped)
    {
        fullnode.update();
        usleep(100000);
    }
    pool.stop();
    pool.join();
    return 0;
}

