#include <iostream>
#include <fstream>
#include <regex>

using namespace std;

int main() {

    size_t max_shards = 3;
    size_t shard_num;

    std::string line;
    std::ifstream infile("db_scripts/data_generation.sql");
    std::ofstream outfile("db_scripts/shard_fill.sql");

    std::string shard_data0 = "use sql_test;\nINSERT INTO Person (login, first_name,last_name,age) VALUES\n";
    std::string shard_data1 = "use sql_test;\nINSERT INTO Person (login, first_name,last_name,age) VALUES\n";
    std::string shard_data2 = "use sql_test;\nINSERT INTO Person (login, first_name,last_name,age) VALUES\n";

    std::smatch match;
    std::regex pattern("([0-9]{3}-[0-9]{2}-[0-9]{4})"); //Pattern for generated login field

    while (std::getline(infile, line)) {
        if (std::regex_search(line, match, pattern)) {
            shard_num = std::hash<std::string>{}(match[0]) % max_shards;
            switch (shard_num) {
                case 0:
                    shard_data0 += line +'\n';
                    break;
                case 1:
                    shard_data1 += line +'\n';
                    break;
                case 2:
                    shard_data2 += line +'\n';
                    break;
            }
        };
    }

    infile.close();

    shard_data0.pop_back();
    shard_data0.pop_back();
    shard_data0 += "\n -- sharding:0\n;\n";
    shard_data1.pop_back();
    shard_data1.pop_back();
    shard_data1 += "\n -- sharding:1\n;\n";
    shard_data2.pop_back();
    shard_data2.pop_back();
    shard_data2 += "\n -- sharding:2\n;\n";

    outfile << shard_data0 << shard_data1 << shard_data2 <<std::endl;
    outfile.close();
    return 0;
} 