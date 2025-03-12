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

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pugixml.hpp>

struct RSSItem {
    std::string title;
    std::string link;
    std::string description;
    std::string pubDate;
    std::string guid;
};

struct RSSChannel {
    std::string title;
    std::string link;
    std::string description;
    std::string language;
    std::string lastBuildDate;
    std::vector<RSSItem> items;
};

class RSSParser {
public:
    [[nodiscard]] std::optional<RSSChannel> parse(
        std::string_view xmlContent) const noexcept
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlContent.data());
        if (!result) {
            std::cerr << "XML Parsing Error: " << result.description() << "\n";
            return std::nullopt;
        }

        const auto channelNode = doc.child("rss").child("channel");
        if (!channelNode) {
            std::cerr << "No <channel> node found. The format may not be "
                         "standard RSS.\n";
            return std::nullopt;
        }

        RSSChannel channel;
        channel.title = channelNode.child("title").text().as_string();
        channel.link = channelNode.child("link").text().as_string();
        channel.description
            = channelNode.child("description").text().as_string();
        channel.language = channelNode.child("language").text().as_string();
        channel.lastBuildDate
            = channelNode.child("lastBuildDate").text().as_string();

        for (const auto& itemNode : channelNode.children("item")) {
            RSSItem item;
            item.title = itemNode.child("title").text().as_string();
            item.link = itemNode.child("link").text().as_string();
            item.description = itemNode.child("description").text().as_string();
            item.pubDate = itemNode.child("pubDate").text().as_string();
            item.guid = itemNode.child("guid").text().as_string();
            channel.items.push_back(std::move(item));
        }
        return channel;
    }
};
