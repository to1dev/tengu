#ifdef USE_TEST

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"

TEST_CASE("Test DotEnv")
{
    SECTION("Load .env")
    {
        auto currentPath = DotEnv::getExePath();
        try {
            auto& parser = DotEnv::getInstance();
            parser.load((currentPath / ".env").string());

            std::string host = parser.getOrDefault("HOST", "localhost");

            REQUIRE(!host.empty());
            REQUIRE(parser.has("API_KEY"));
        } catch (const DotEnvException& e) {
            std::cerr << "Configuration error: " << e.what() << std::endl;
        }
    }
}

#endif
