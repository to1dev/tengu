// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <chrono>
#include <iostream>

#include "catch_amalgamated.hpp"

#include "Utils/Dotenv.hpp"
#include "Utils/PathUtils.hpp"

using namespace Daitengu::Utils;

TEST_CASE("Test DotEnv")
{
    SECTION("Clock test")
    {
        using namespace std::chrono;

        steady_clock::time_point t1 = steady_clock::now();
        for (volatile int i = 0; i < 1000; ++i)
            ;
        steady_clock::time_point t2 = steady_clock::now();

        auto duration = duration_cast<nanoseconds>(t2 - t1).count();
        std::cout << "Duration: " << duration << " ns" << std::endl;
    }

    SECTION("Load .env")
    {
        auto currentPath = PathUtils::getExecutableDir();
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

int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);

    return result;
}
