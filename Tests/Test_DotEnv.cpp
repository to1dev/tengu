#ifdef USE_TEST

#include <filesystem>

#include <windows.h>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"

std::filesystem::path getExePath()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#endif
}

TEST_CASE("Test DotEnv")
{

    SECTION("Load .env")
    {
        auto currentPath = getExePath();
        try {
            auto& parser = DotEnv::getInstance();
            parser.load((currentPath / ".env").string());

            if (auto value = parser.get("MNEMONIC")) {
                std::cout << "Mnemonic: " << *value << std::endl;
            }

            std::string host = parser.getOrDefault("HOST", "localhost");

            std::cout << "Host is: " << host << std::endl;

            REQUIRE(parser.has("API_KEY"));
        } catch (const DotEnvException& e) {
            std::cerr << "Configuration error: " << e.what() << std::endl;
        }
    }
}

#endif
