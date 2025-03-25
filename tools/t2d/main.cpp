#include <filesystem>
#include <fstream>
#include <iostream>
#include <lmdb.h>
#include <simdjson.h>

constexpr const char* JSON_FILE_PATH = "tokens.json";
constexpr const char* LMDB_PATH = "./lmdb_data";

void die(const std::string& msg, int rc)
{
    std::cerr << msg << " (code " << rc << ")\n";
    exit(1);
}

int main(int argc, char** argv)
{
    bool clear_db = true;
    int map_size_mb = 512;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--clear") {
            clear_db = true;
        } else if (arg == "--size" && i + 1 < argc) {
            map_size_mb = std::stoi(argv[++i]);
        }
    }

    if (!std::filesystem::exists(LMDB_PATH)) {
        std::filesystem::create_directories(LMDB_PATH);
    }

    if (clear_db && std::filesystem::exists(LMDB_PATH)) {
        std::cout << "Clearing database directory...\n";
        std::filesystem::remove_all(LMDB_PATH);
        std::filesystem::create_directories(LMDB_PATH);
    }

    simdjson::ondemand::parser parser;

    std::ifstream file(JSON_FILE_PATH);
    if (!file.is_open())
        die("Failed to open JSON file", -1);
    std::string json_data((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    auto doc = parser.iterate(json_data);
    auto array = doc.get_array();

    MDB_env* env;
    MDB_dbi dbi;
    MDB_txn* txn;
    MDB_val key, data;

    int rc = mdb_env_create(&env);
    if (rc != 0)
        die("Failed to create LMDB environment", rc);

    mdb_env_set_mapsize(env, static_cast<size_t>(map_size_mb) * 1024 * 1024);

    rc = mdb_env_open(env, LMDB_PATH, 0, 0664);
    if (rc != 0)
        die("Failed to open LMDB environment", rc);

    rc = mdb_txn_begin(env, nullptr, 0, &txn);
    if (rc != 0)
        die("Failed to begin transaction", rc);

    rc = mdb_dbi_open(txn, nullptr, 0, &dbi);
    if (rc != 0)
        die("Failed to open LMDB database", rc);

    if (clear_db) {
        rc = mdb_drop(txn, dbi, 0);
        if (rc != 0)
            die("Failed to clear database", rc);
    }

    size_t count = 0;
    size_t skipped = 0;
    size_t errors = 0;

    for (auto obj : array) {
        try {
            std::string address
                = std::string(obj["address"].get_string().value());
            std::string obj_json
                = std::string(simdjson::to_json_string(obj).value());

            key.mv_size = address.size();
            key.mv_data = address.data();
            data.mv_size = obj_json.size();
            data.mv_data = (void*)obj_json.data();

            rc = mdb_put(txn, dbi, &key, &data, 0);

            if (rc != 0) {
                std::cerr << "Insert failed: " << address << " (code " << rc
                          << ")\n";
                errors++;

                if (rc == MDB_BAD_TXN) {
                    mdb_txn_abort(txn);

                    rc = mdb_txn_begin(env, nullptr, 0, &txn);
                    if (rc != 0)
                        die("Failed to restart transaction", rc);

                    rc = mdb_dbi_open(txn, nullptr, 0, &dbi);
                    if (rc != 0)
                        die("Failed to reopen database", rc);

                    rc = mdb_put(txn, dbi, &key, &data, 0);
                    if (rc == 0) {
                        count++;
                    } else {
                        errors++;
                    }
                }
            } else {
                count++;
            }

            if ((count + errors + skipped) % 1000 == 0) {
                std::cout << "Progress: " << count << " records inserted\n";
            }
        } catch (const simdjson::simdjson_error& e) {
            std::cerr << "Skipped an item due to JSON parsing error: "
                      << e.what() << "\n";
            skipped++;
        }
    }

    rc = mdb_txn_commit(txn);
    if (rc != 0)
        die("Failed to commit transaction", rc);

    mdb_dbi_close(env, dbi);
    mdb_env_close(env);

    std::cout << "Finished. " << count << " records written, " << skipped
              << " skipped, " << errors << " errors.\n";

    return 0;
}
