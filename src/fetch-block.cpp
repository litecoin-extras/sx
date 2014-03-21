#include <bitcoin/bitcoin.hpp>
#include <obelisk/obelisk.hpp>
#include "config.hpp"
#include "util.hpp"

using namespace bc;
using std::placeholders::_1;
using std::placeholders::_2;

bool stopped = false;
size_t height = 0;
bool json_output = false;

void block_fetched(const std::error_code& ec,
    const hash_digest_list& blk)
{
    if (ec)
    {
        std::cerr << "fetch-block: " << ec.message() << std::endl;
        stopped = true;
        return;
    }

    if (json_output){
        bool is_first = true;
        std::cout << "{" << std::endl;
        std::cout << "  \"block\": \"" << height << "\"," << std::endl;
        std::cout << "  \"transactions\": [" << std::endl;
        for (const hash_digest& row: blk){
            std::cout << "    ";
            if(is_first == false)
                std::cout << ",";
            if (is_first)
                is_first = false;

            std::cout <<"\"" << row << "\"" << std::endl;
        }
        std::cout << "  ]\n}" << std::endl;
    }else{
        std::cout << "block: " << height << std::endl;
        for (const hash_digest& row: blk)
            std::cout << " tx_hash: " <<  row << std::endl;
    }
    stopped = true;
}

int main(int argc, char** argv)
{
    std::string index_str="0";
    size_t height = 0;

    
    for (size_t i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "-j" || arg == "--json")
        {
            json_output = true;
            continue;
        }
        else{
            index_str = argv[i];
        }

    }

    if (index_str == "0")
    {
       std::cerr << "Usage: sx fetch-block [-j] BLOCK-HEIGHT" << std::endl;
       return -1;
    }
    
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
    fullnode.blockchain.fetch_block(height, block_fetched);
    while (!stopped)
    {
        fullnode.update();
        usleep(100000);
    }
    pool.stop();
    pool.join();
    return 0;
}

