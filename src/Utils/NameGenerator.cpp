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

#include "NameGenerator.h"

namespace Daitengu::Utils {

static std::mt19937 rng(
    std::chrono::high_resolution_clock::now().time_since_epoch().count());

std::wstring towstring(const std::string& s)
{
    if (s.empty()) {
        return std::wstring();
    }

    std::mbstate_t state {};
    const char* src = s.c_str();
    std::size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (len == static_cast<std::size_t>(-1)) {
        return std::wstring();
    }

    std::vector<wchar_t> buf(len + 1, L'\0');
    src = s.c_str();
    std::mbsrtowcs(buf.data(), &src, len, &state);
    return std::wstring(buf.data());
}

std::string tostring(const std::wstring& ws)
{
    if (ws.empty()) {
        return std::string();
    }

    std::mbstate_t state {};
    const wchar_t* src = ws.c_str();
    std::size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (len == static_cast<std::size_t>(-1)) {
        return std::string();
    }

    std::vector<char> buf(len + 1, '\0');
    src = ws.c_str();
    std::wcsrtombs(buf.data(), &src, len, &state);
    return std::string(buf.data());
}

NameGenerator::NameGenerator() = default;

NameGenerator::NameGenerator(
    std::vector<std::unique_ptr<NameGenerator>>&& generators_)
    : generators(std::move(generators_))
{
}

NameGenerator::NameGenerator(const std::string& pattern, bool collapse_triples)
{
    std::unique_ptr<NameGenerator> last;

    // We parse the pattern using Group stacks
    std::stack<std::unique_ptr<Group>> parseStack;
    std::unique_ptr<Group> top
        = std::make_unique<GroupSymbol>(); // initial: symbol group

    for (char c : pattern) {
        switch (c) {
        case '<':
            parseStack.push(std::move(top));
            top = std::make_unique<GroupSymbol>();
            break;
        case '(':
            parseStack.push(std::move(top));
            top = std::make_unique<GroupLiteral>();
            break;
        case '>':
        case ')':
            if (parseStack.empty()) {
                throw std::invalid_argument(
                    "Unbalanced brackets: extra '>' or ')'");
            }
            if (c == '>' && top->type != group_types::symbol) {
                throw std::invalid_argument(
                    "Unexpected '>' outside symbol group");
            }
            if (c == ')' && top->type != group_types::literal) {
                throw std::invalid_argument(
                    "Unexpected ')' outside literal group");
            }
            last = top->produce();
            top = std::move(parseStack.top());
            parseStack.pop();
            top->add(std::move(last));
            break;
        case '|':
            top->split();
            break;
        case '!':
            if (top->type == group_types::symbol) {
                top->wrap(wrappers::capitalizer);
            } else {
                // If inside literal, treat '!' as normal char
                top->add(c);
            }
            break;
        case '~':
            if (top->type == group_types::symbol) {
                top->wrap(wrappers::reverser);
            } else {
                top->add(c);
            }
            break;
        default:
            top->add(c);
            break;
        }
    }

    if (!parseStack.empty()) {
        throw std::invalid_argument("Unbalanced brackets: missing '>' or ')'");
    }

    std::unique_ptr<NameGenerator> g = top->produce();
    if (collapse_triples) {
        g = std::make_unique<Collapser>(std::move(g));
    }
    add(std::move(g));
}

std::size_t NameGenerator::combinations()
{
    std::size_t total = 1;
    for (auto& g : generators) {
        total *= g->combinations();
    }
    return total;
}

std::size_t NameGenerator::min()
{
    std::size_t total = 0;
    for (auto& g : generators) {
        total += g->min();
    }
    return total;
}

std::size_t NameGenerator::max()
{
    std::size_t total = 0;
    for (auto& g : generators) {
        total += g->max();
    }
    return total;
}

std::string NameGenerator::toString()
{
    // In the base case, we just concatenate the results of child generators
    std::string result;
    for (auto& g : generators) {
        result += g->toString();
    }
    return result;
}

void NameGenerator::add(std::unique_ptr<NameGenerator>&& g)
{
    generators.push_back(std::move(g));
}

NameGenerator::Group::Group(group_types t)
    : type(t)
{
}

std::unique_ptr<NameGenerator> NameGenerator::Group::produce()
{
    if (set.empty()) {
        // No children => return empty literal
        return std::make_unique<Literal>("");
    }
    if (set.size() == 1) {
        // If only one child, pass it through
        return std::move(set[0]);
    }
    // Otherwise, random choice among them
    return std::make_unique<Random>(std::move(set));
}

void NameGenerator::Group::split()
{
    // Insert a new sequence so that next items go there
    set.push_back(std::make_unique<Sequence>());
}

void NameGenerator::Group::wrap(wrappers w)
{
    wrapperStack.push(w);
}

void NameGenerator::Group::add(std::unique_ptr<NameGenerator>&& g)
{
    // If we have any pending wrappers, apply them first
    while (!wrapperStack.empty()) {
        switch (wrapperStack.top()) {
        case wrappers::capitalizer:
            g = std::make_unique<Capitalizer>(std::move(g));
            break;
        case wrappers::reverser:
            g = std::make_unique<Reverser>(std::move(g));
            break;
        }
        wrapperStack.pop();
    }

    if (set.empty()) {
        // Ensure we have at least one Sequence to add into
        set.push_back(std::make_unique<Sequence>());
    }
    // Add the generator to the last sequence
    set.back()->add(std::move(g));
}

void NameGenerator::Group::add(char c)
{
    // For single char, wrap in a Random + Literal
    std::string tmp(1, c);
    auto r = std::make_unique<Random>();
    r->add(std::make_unique<Literal>(tmp));
    add(std::move(r));
}

NameGenerator::GroupSymbol::GroupSymbol()
    : Group(group_types::symbol)
{
}

void NameGenerator::GroupSymbol::add(char c)
{
    std::string key(1, c);
    auto it = SymbolMap().find(key);
    if (it == SymbolMap().end()) {
        // Not in symbol map => treat as literal char
        Group::add(c);
        return;
    }
    // If found, create a random generator containing all mapped expansions
    auto r = std::make_unique<Random>();
    for (auto& val : it->second) {
        r->add(std::make_unique<Literal>(val));
    }
    // Now pass r to base add => could apply wrappers, etc.
    Group::add(std::move(r));
}

NameGenerator::GroupLiteral::GroupLiteral()
    : Group(group_types::literal)
{
}

const std::unordered_map<std::string, const std::vector<std::string>>&
NameGenerator::SymbolMap()
{
    static auto* const symbols = new std::unordered_map<std::string,
        const std::vector<std::string>>({
        { "s",
            { "ach", "ack", "ad", "age", "ald", "ale", "an", "ang", "ar", "ard",
                "as", "ash", "at", "ath", "augh", "aw", "ban", "bel", "bur",
                "cer", "cha", "che", "dan", "dar", "del", "den", "dra", "dyn",
                "ech", "eld", "elm", "em", "en", "end", "eng", "enth", "er",
                "ess", "est", "et", "gar", "gha", "hat", "hin", "hon", "ia",
                "ight", "ild", "im", "ina", "ine", "ing", "ir", "is", "iss",
                "it", "kal", "kel", "kim", "kin", "ler", "lor", "lye", "mor",
                "mos", "nal", "ny", "nys", "old", "om", "on", "or", "orm", "os",
                "ough", "per", "pol", "qua", "que", "rad", "rak", "ran", "ray",
                "ril", "ris", "rod", "roth", "ryn", "sam", "say", "ser", "shy",
                "skel", "sul", "tai", "tan", "tas", "ther", "tia", "tin", "ton",
                "tor", "tur", "um", "und", "unt", "urn", "usk", "ust", "ver",
                "ves", "vor", "war", "wor", "yer" } },
        { "v", { "a", "e", "i", "o", "u", "y" } },
        { "V",
            { "a", "e", "i", "o", "u", "y", "ae", "ai", "au", "ay", "ea", "ee",
                "ei", "eu", "ey", "ia", "ie", "oe", "oi", "oo", "ou", "ui" } },
        { "c",
            { "b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q",
                "r", "s", "t", "v", "w", "x", "y", "z" } },
        { "B",
            { "b", "bl", "br", "c", "ch", "chr", "cl", "cr", "d", "dr", "f",
                "g", "h", "j", "k", "l", "ll", "m", "n", "p", "ph", "qu", "r",
                "rh", "s", "sch", "sh", "sl", "sm", "sn", "st", "str", "sw",
                "t", "th", "thr", "tr", "v", "w", "wh", "y", "z", "zh" } },
        { "C",
            { "b", "c", "ch", "ck", "d", "f", "g", "gh", "h", "k", "l", "ld",
                "ll", "lt", "m", "n", "nd", "nn", "nt", "p", "ph", "q", "r",
                "rd", "rr", "rt", "s", "sh", "ss", "st", "t", "th", "v", "w",
                "y", "z" } },
        { "i",
            { "air", "ankle", "ball", "beef", "bone", "bum", "bumble", "bump",
                "cheese", "clod", "clot", "clown", "corn", "dip", "dolt",
                "doof", "dork", "dumb", "face", "finger", "foot", "fumble",
                "goof", "grumble", "head", "knock", "knocker", "knuckle",
                "loaf", "lump", "lunk", "meat", "muck", "munch", "nit", "numb",
                "pin", "puff", "skull", "snark", "sneeze", "thimble", "twerp",
                "twit", "wad", "wimp", "wipe" } },
        { "m",
            { "baby", "booble", "bunker", "cuddle", "cuddly", "cutie", "doodle",
                "foofie", "gooble", "honey", "kissie", "lover", "lovey",
                "moofie", "mooglie", "moopie", "moopsie", "nookum", "poochie",
                "poof", "poofie", "pookie", "schmoopie", "schnoogle",
                "schnookie", "schnookum", "smooch", "smoochie", "smoosh",
                "snoogle", "snoogy", "snookie", "snookum", "snuggy", "sweetie",
                "woogle", "woogy", "wookie", "wookum", "wuddle", "wuddly",
                "wuggy", "wunny" } },
        { "M",
            { "boo", "bunch", "bunny", "cake", "cakes", "cute", "darling",
                "dumpling", "dumplings", "face", "foof", "goo", "head", "kin",
                "kins", "lips", "love", "mush", "pie", "poo", "pooh", "pook",
                "pums" } },
        { "D",
            { "b", "bl", "br", "cl", "d", "f", "fl", "fr", "g", "gh", "gl",
                "gr", "h", "j", "k", "kl", "m", "n", "p", "th", "w" } },
        { "d",
            { "elch", "idiot", "ob", "og", "ok", "olph", "olt", "omph", "ong",
                "onk", "oo", "oob", "oof", "oog", "ook", "ooz", "org", "ork",
                "orm", "oron", "ub", "uck", "ug", "ulf", "ult", "um", "umb",
                "ump", "umph", "un", "unb", "ung", "unk", "unph", "unt",
                "uzz" } },

        { "W",
            { "Stonehammer", "Thunderaxe", "Shadowfang", "Ironskull",
                "Windwhisperer", "Bloodhoof", "Firefist", "Bonecrusher",
                "Frostblade", "Soulwalker", "Skullsplitter", "Destroyer",
                "Frostwolf", "Stormfury", "Flameheart", "Giantmaul",
                "Nightshade", "Mistfang", "Frostmane", "Stormcaller",
                "Blackclaw", "Ironback", "BreathoftheDeep", "Starbreaker",
                "Darkeye", "Sunscorch", "Wildflame", "Venomsting",
                "Whitefeather", "Backbreaker", "Sunseeker", "Darkhorn",
                "Bloodeye", "Frostspeaker", "Ironhorn", "Thunderhowl",
                "Hellflame", "Nightmarecaller", "Shadowveil", "Silvershield",
                "Windrage", "Fangstorm", "Savageaxe", "Morningstar",
                "Spiritflame", "Burninghorn", "Giantflame", "Soulslicer",
                "Sealer", "Bloodshadow", "Tigerfang", "Dawnbreaker",
                "Blazefury", "Bloodfire", "Terrorclaw", "Blackheart", "Skyoath",
                "Spiritwhisperer", "Stoneshatterer", "Hellwalker", "Demonhorn",
                "Deathwhisperer", "Blackwing", "Nightcryer", "Starscar",
                "Eagleeye", "Redfeather", "Venomspitter", "Bonecrag",
                "Twilight", "Stormhoof", "Snowrage", "Bloodwolf", "Darkflame",
                "Starwrath", "Wastelandwalker", "Blizzard", "Blueflame",
                "Rockbreaker", "Hellfire", "Desertfang", "Galehowl",
                "Blazeblood", "Shadowrage", "Demonspirit", "Starpit",
                "Frostbreath", "Deathsoul", "Skyfeather", "Emberblaze",
                "Demontrot", "Earthshaker", "Shadowstalker", "Sandfury",
                "Blueburn", "Ghosthowl", "Toxicmist", "Darktide", "Stormtongue",
                "Skyscar", "Spiriteye", "Bonesoul", "Stormbreath", "Bloodhorn",
                "Ironbreaker", "Stonechant", "Wolfstalker", "Darkhowl",
                "Venomburst", "Nighthowl", "Earthblade", "Ashchant",
                "Sunwatcher", "Thunderroar", "Earthmark", "Ironeye",
                "Shadowhorn", "Skywatcher", "Sunclaw", "Stonepath", "Soulblade",
                "Dawnoath", "Spiritscar", "Skystalker", "Echobreaker",
                "Thunderscar", "Spiritpath", "Wolfgleam", "Darkfang", "Icefire",
                "Spiritcurse", "Soulrage", "Wolfbane", "Stormgleam", "Skymark",
                "Stonecurse", "Shadowbreaker", "Spiritstalker", "Ashwrath",
                "Mistwalker", "Shadowoath", "Ashcoil", "Windgleam",
                "Ghostdrifter", "Flamewalker", "Ironcurse", "Stormhorn",
                "Earthpath", "Venomoath", "Thunderbreaker", "Wolfseeker",
                "Ashhowl", "Shadowwalker", "Ashwatcher", "Hellfang", "Starsoul",
                "Venomchant", "Hellcaller", "Icebreaker", "Windpath",
                "Mistgloom", "Bonebreaker", "Venomgloom", "Spiritwalker",
                "Flameshade", "Icehorn", "Wolfcoil", "Thunderfang", "Darkgleam",
                "Staroath", "Moonbane", "Dawncaller", "Thunderdream",
                "Starrage", "Shadowbreath", "Skyhowl", "Hellgloom", "Skybreath",
                "Stormgloom", "Sunrage", "Windeye", "Stormchant", "Stareye",
                "Stonewrath", "Firevoice", "Ashfire", "Bloodhowl", "Shadowbane",
                "Echoroar", "Darkwatcher", "Earthburst", "Stardrifter",
                "Earthbane", "Ghostwalker", "Nightroar", "Dawnburst",
                "Echogleam", "Moonwrath", "Iceclaw", "Echocaller", "Icedream",
                "Dawnsinger", "Flamemark", "Thunderwatcher", "Flamefire",
                "Bonevoice", "Mistseeker", "Icesnarl", "Stonesoul",
                "Firecaller", "Wolfsnarl", "Iceburst", "Stormvoice",
                "Shadowdream", "Starwalker", "Bonebreath", "Ashoath",
                "Stormwatcher", "Frostbane", "Stonecaller", "Ashbreath",
                "Earthgloom", "Moonpath", "Frostfury", "Spiritcrusher",
                "Skygleam", "Windoath", "Frostwalker", "Echosoul", "Moondream",
                "Nightcaller", "Venomwalker", "Flameclaw", "Deathbane",
                "Shadowchant", "Bonemaw", "Shadowfury", "Echoblade",
                "Mistgleam", "Mooncaller", "Ironmaw", "Shadowshade",
                "Echodrifter", "Starvoice", "Hellfury", "Bonegleam", "Moonhowl",
                "Mistchant", "Spiritgleam", "Darkfire", "Deathburst",
                "Thunderstalker", "Nightscar", "Stoneblade", "Echochant",
                "Souldrifter", "Boneroar", "Frostshade", "Earthsinger",
                "Firegloom", "Soulsoul", "Soulhowl", "Frostcaller", "Frostclaw",
                "Souloath", "Ironscar", "Bonerage", "Stonewalker", "Dawnmaw",
                "Echodream", "Ghosteye", "Darkcaller", "Venomfang", "Soulmaw",
                "Ashbreaker", "Ghostbreath", "Earthhowl", "Bloodscar",
                "Bloodmaw", "Wolfwalker", "Bloodburst", "Bonedream", "Bonebane",
                "Venomgleam", "Dawnvoice", "Icevoice", "Starmark", "Venomhowl",
                "Firemaw", "Moonblade", "Firestalker", "Wolfburst",
                "Stormblade", "Bloodblade", "Ashhorn", "Mistcrusher",
                "Echohowl", "Wolfoath", "Stormfang", "Wolfmark", "Frostcoil",
                "Wolfcaller", "Stonerage", "Nightclaw", "Icecoil", "Frostsnarl",
                "Darkwalker", "Bloodoath", "Thunderfury", "Ironsinger",
                "Skyclaw", "Bloodbreaker", "Starcrusher", "Nightcoil",
                "Nightdream", "Echowalker", "Mistwatcher", "Hellbreaker",
                "Ashcaller", "Darkoath", "Spiritbreath", "Ghosthorn",
                "Boneclaw", "Ashgloom", "Deathmaw", "Shadowroar",
                "Thunderchant", "Nighteye", "Bloodwrath", "Icefang", "Echorage",
                "Windhowl", "Sunbreath", "Hellcoil", "Iceshade", "Shadowscar",
                "Icesinger", "Windwrath", "Windburst", "Shadowblade",
                "Iceblade", "Stonefire", "Spiritsnarl", "Soulgleam",
                "Moonstalker", "Bloodclaw", "Starcurse", "Souldream",
                "Darkdream", "Soulchant", "Darkscar", "Windsinger",
                "Venomdrifter", "Icewalker", "Thunderrage", "Sunsnarl",
                "Echoeye", "Thundercaller", "Stormstalker", "Sunroar",
                "Flamesoul", "Nightbane", "Soulseeker", "Hellmaw", "Earthcurse",
                "Ironfang", "Ironburst", "Spiritblade", "Spiritburst",
                "Venomcurse", "Deathcoil", "Hellpath", "Wolfbreath",
                "Skywalker", "Spiritcaller", "Firehorn", "Nightwalker",
                "Mistroar", "Bloodwalker", "Mistmaw", "Firesnarl", "Ashshade",
                "Thundergleam", "Soulroar", "Ghostfang", "Windwatcher",
                "Darkseeker", "Nightfire", "Moonseeker", "Wolfwatcher",
                "Moonoath", "Echofury", "Frostchant", "Hellhorn", "Soulstalker",
                "Nightseeker", "Hellscar", "Bloodfury", "Deathwatcher",
                "Echoshade", "Bloodbane", "Nightcrusher", "Moonwalker",
                "Darkfury", "Deathwrath", "Dawnstalker", "Darkdrifter",
                "Stormsoul", "Earthshade", "Earthcrusher", "Stonebreath",
                "Windstalker", "Ironhowl", "Deathhorn", "Stormcurse",
                "Dawnbreath", "Deathshade", "Nightwatcher", "Ghostblade",
                "Firemark", "Mooneye", "Stoneclaw", "Firebane", "Bloodsnarl",
                "Hellcurse", "Deathseeker", "Spirithorn", "Earthcaller",
                "Starhowl", "Bonecaller", "Fireburst", "Mistsinger",
                "Spiritsoul", "Hellchant", "Soulcrusher", "Hellroar",
                "Firechant", "Dawnseeker", "Skyburst", "Firewalker",
                "Frostvoice", "Nightvoice", "Echobane", "Mistfury",
                "Frostgleam", "Wolfroar", "Ghostseeker", "Firepath",
                "Mooncurse", "Starsnarl", "Ironsoul", "Shadowvoice",
                "Windsnarl", "Darkshade", "Froststalker", "Windbreath",
                "Starpath", "Nightsoul", "Earthscar", "Thundercurse",
                "Bloodcoil", "Starchant", "Darkcurse", "Firesoul", "Skychant",
                "Moonhorn", "Ironsnarl", "Shadowsoul", "Nightbreaker",
                "Ashsoul", "Spiritfury", "Earthwatcher", "Stargleam",
                "Skyblade", "Ghostmaw", "Flameoath", "Hellrage", "Nightmark",
                "Firescar", "Wolfdrifter", "Soulfury", "Ironseeker",
                "Stoneoath", "Earthfury", "Flamesnarl", "Irondrifter",
                "Ironrage", "Skywrath", "Soulshade", "Bloodrage", "Boneeye",
                "Wolfbreaker", "Soulbreaker", "Ironchant", "Spiritoath",
                "Soulmark", "Bonehorn", "Mooncrusher", "Fireblade",
                "Frostbreaker", "Stargloom", "Deathfang", "Bonesinger",
                "Mistfire", "Thunderburst", "Spirithowl", "Windcoil",
                "Helldream", "Starstalker", "Dawnfire", "Sunchant", "Skycurse",
                "Hellsoul", "Stormburst", "Skysoul", "Skyeye", "Deathfire",
                "Windfang", "Windcurse", "Stonedream", "Deathhowl",
                "Nightgleam", "Hellwatcher", "Shadowgloom", "Mooncoil",
                "Stonefury", "Mistbane", "Earthfang", "Deathfury", "Nightfang",
                "Stonesinger", "Frosthorn", "Soulscar", "Ghostvoice",
                "Bonewalker", "Stonehorn", "Icebane", "Dawnchant", "Icecrusher",
                "Bonehowl", "Mistblade", "Earthcoil", "Thunderhorn", "Soulcoil",
                "Sundrifter", "Nightchant", "Thunderclaw", "Dawnhorn",
                "Venommark", "Ashfang", "Frostpath", "Stormscar", "Darksnarl",
                "Spiritfire", "Skypath", "Iceseeker", "Echostalker",
                "Venomwrath", "Ghostgleam", "Echogloom", "Helldrifter",
                "Bloodchant", "Soulbane", "Moonmaw", "Ashscar", "Darkburst",
                "Moonsinger", "Ghostchant", "Flamevoice", "Shadowpath",
                "Soulpath", "Hellseeker", "Hellstalker", "Stonehowl",
                "Ironcoil", "Wolfcurse", "Earthroar", "Starcoil", "Blooddream",
                "Firefang", "Icehowl", "Nightbreath", "Bloodshade",
                "Ghostdream", "Wolfvoice", "Ashburst", "Darkcoil",
                "Moonwatcher", "Soulgloom", "Shadowcaller", "Stonefang",
                "Ashsnarl", "Ironcrusher", "Ghostburst", "Earthfire",
                "Ashgleam", "Windshade", "Firehowl", "Venomrage", "Mistrage",
                "Bonecurse", "Shadowmark", "Ashcrusher", "Wolfsinger",
                "Thundercrusher", "Ghostcaller", "Fireclaw", "Skybreaker",
                "Venomcrusher", "Moonvoice", "Wolfhowl", "Windbreaker",
                "Moongleam", "Deathchant", "Misthowl", "Flamefury", "Icegleam",
                "Spiritroar", "Sunpath", "Ashbane", "Stonebane", "Flamefang",
                "Soulfang", "Ashpath", "Bloodpath", "Starfire", "Dawnmark",
                "Hellsnarl", "Earthsnarl", "Hellburst", "Icecurse", "Flamepath",
                "Iceeye", "Suncurse", "Spiritseeker", "Flameburst",
                "Earthseeker", "Moonfang", "Skyvoice", "Venomseeker",
                "Stormsinger", "Stormclaw", "Ashrage", "Earthoath", "Skyshade",
                "Flameseeker", "Dawneye", "Dawnwrath", "Windchant",
                "Bonewatcher", "Stoneroar", "Icepath", "Shadowseeker",
                "Deathmark", "Starburst", "Ghostfire", "Firegleam",
                "Dawnwalker", "Wolfdream", "Echofang", "Spiritgloom",
                "Starcaller", "Frosthowl", "Frostgloom", "Venomroar", "Iceroar",
                "Spiritbane", "Deathcurse", "Sunfire", "Wolfeye", "Echocurse",
                "Wolfshade", "Deathwalker", "Fireseeker", "Flamerage",
                "Moonsnarl", "Firecoil", "Earthbreaker", "Spiritmaw",
                "Flamebreath", "Deathscar", "Firecrusher", "Flamewrath",
                "Deathroar", "Starblade", "Deathsinger", "Dawnshade",
                "Ghostgloom", "Suncaller", "Deathbreath", "Ghostroar",
                "Darkroar", "Spiritclaw", "Venomclaw", "Moonfire", "Echohorn",
                "Mistsnarl", "Sunmaw", "Soulburst", "Boneshade", "Shadowsnarl",
                "Windcrusher", "Darkmark", "Dawngleam", "Nightgloom",
                "Bloodgleam", "Starshade", "Dawnfury", "Thunderblade",
                "Stardream", "Spiritdrifter", "Stormhowl", "Mistbreath",
                "Windseeker", "Venomsinger", "Skymaw", "Windwalker", "Icescar",
                "Darkbreath", "Mistdream", "Thundercoil", "Ironmark",
                "Soulbreath", "Windmaw", "Starmaw", "Thundersinger",
                "Shadowgleam", "Echosinger", "Ghostmark", "Ironwatcher",
                "Deathdream", "Ironcaller", "Firewatcher", "Windroar",
                "Dawnpath", "Ghoststalker", "Icestalker", "Stormwalker",
                "Darkgloom", "Hellshade", "Stonewatcher", "Moonclaw",
                "Earthvoice", "Venombane", "Bloodsoul", "Darkwrath",
                "Venomshade", "Stonestalker", "Earthwalker", "Icechant",
                "Darkrage", "Ghostcrusher", "Starsinger", "Moonmark",
                "Nightmaw", "Nightsinger", "Ashroar", "Bonefire", "Venomvoice",
                "Starfang", "Venomcaller", "Soulclaw", "Soulcaller",
                "Shadowburst", "Ironbreath", "Mistclaw", "Sunblade", "Bonemark",
                "Bonefang", "Soulhorn", "Darkvoice", "Starbane", "Venommaw",
                "Soulfire", "Dawncrusher", "Wolffury", "Dawncurse",
                "Deathblade", "Ashmaw", "Venomblade", "Earthrage", "Fireoath",
                "Bloodbreath", "Deathvoice", "Nightdrifter", "Boneseeker",
                "Darkpath", "Earthhorn", "Flamemaw", "Ghostbreaker",
                "Hellwrath", "Boneblade", "Moondrifter", "Boneoath",
                "Ghostshade", "Bloodseeker", "Bonechant", "Ironvoice",
                "Mistdrifter", "Moonbreaker", "Bloodroar", "Deathclaw",
                "Icewrath", "Firebreaker", "Dawnclaw", "Echobreath",
                "Ghostbane", "Stormcrusher", "Wolfgloom", "Skycoil", "Sundream",
                "Nightrage", "Flamescar", "Dawndrifter", "Thunderbane",
                "Spiritmark", "Stonemaw", "Sunhorn", "Echoburst", "Dawnwatcher",
                "Ghostscar", "Windclaw", "Bloodcaller", "Frostwrath",
                "Venomstalker", "Ghostpath", "Darkchant", "Stormfire",
                "Ironblade", "Moonroar", "Bonestalker", "Spiritdream",
                "Deathdrifter", "Fireshade", "Flamechant", "Firebreath",
                "Sunstalker", "Firefury", "Ironfire", "Blooddrifter", "Sunmark",
                "Darksoul", "Ironclaw", "Spiritrage", "Sunwalker", "Mistburst",
                "Flamecoil", "Stormdrifter", "Shadowfire", "Sungloom",
                "Ashcurse", "Stonecrusher", "Nightwrath", "Starclaw",
                "Darkbane", "Miststalker", "Icemark", "Frostseeker",
                "Thunderfire", "Soulsnarl", "Nightburst", "Stonedrifter",
                "Hellbane", "Thundereye", "Windscar", "Frostdream", "Skycaller",
                "Spiritcoil", "Ironroar", "Ashclaw", "Bonefury", "Suneye",
                "Misthorn", "Shadowcrusher", "Winddrifter", "Icerage",
                "Ironbane", "Thunderpath", "Suncoil", "Echoseeker", "Deathpath",
                "Sunshade", "Sungleam", "Stormpath", "Boneburst", "Hellgleam",
                "Deathcaller", "Flamegleam", "Flamesinger", "Helloath",
                "Ironoath", "Windhorn", "Ironfury", "Hellhowl", "Stormmaw",
                "Frostsinger", "Sunburst", "Deatheye", "Earthbreath",
                "Ironshade", "Wolfscar", "Ghostcurse", "Dawnfang", "Echomark",
                "Darkblade", "Mistbreaker", "Thunderoath", "Earthmaw",
                "Deathoath", "Soulcurse", "Hellvoice", "Icedrifter",
                "Stonemark", "Stonegleam", "Firesinger", "Frostmaw",
                "Shadowdrifter", "Firedream", "Moonscar", "Dawnroar",
                "Stoneeye", "Mistsoul", "Echowrath", "Starroar", "Skyfire",
                "Moonfury", "Wolfpath", "Echosnarl", "Stonebreaker",
                "Ghostsoul", "Ashfury", "Echowatcher", "Darkmaw", "Skysinger",
                "Hellsinger", "Darkbreaker", "Spiritbreaker", "Deathgleam",
                "Sunwrath", "Earthsoul", "Mistcaller", "Stormeye", "Stormcoil",
                "Sunsinger", "Starbreath", "Venomcoil", "Windsoul", "Fireroar",
                "Frostcrusher", "Flamedrifter", "Ashdream", "Venomwatcher",
                "Bloodvoice", "Venompath", "Ashwalker", "Darkclaw", "Ashseeker",
                "Venomscar", "Frostfire", "Dawncoil", "Sunvoice", "Misteye",
                "Nightsnarl", "Dawnhowl", "Darkcrusher", "Flameblade",
                "Thundershade", "Echomaw", "Stormbane", "Venomsoul", "Windbane",
                "Ironwalker", "Fireeye", "Frosteye", "Venomdream", "Flamedream",
                "Ashblade", "Mistcoil", "Ironwrath", "Wolfclaw", "Wolfmaw",
                "Darksinger", "Venomhorn", "Wolfblade", "Ghostfury",
                "Bloodstalker", "Frostsoul", "Stormmark", "Skycrusher",
                "Nightblade", "Shadowmaw", "Skyrage", "Flamebreaker",
                "Thundersnarl", "Mistoath", "Shadowclaw", "Shadowcurse",
                "Venomfury", "Flamegloom", "Sunscar" } },
    });
    return *symbols;
}

Random::Random(std::vector<std::unique_ptr<NameGenerator>>&& generators_)
    : NameGenerator(std::move(generators_))
{
}

std::size_t Random::combinations()
{
    // Random: sum of children combos
    std::size_t total = 0;
    for (auto& g : generators) {
        total += g->combinations();
    }
    return total == 0 ? 1 : total;
}

std::size_t Random::min()
{
    // Return the minimum 'min()' among children
    if (generators.empty()) {
        return 0;
    }
    std::size_t res = static_cast<std::size_t>(-1);
    for (auto& g : generators) {
        std::size_t m = g->min();
        if (m < res) {
            res = m;
        }
    }
    return res;
}

std::size_t Random::max()
{
    // Return the maximum 'max()' among children
    std::size_t res = 0;
    for (auto& g : generators) {
        std::size_t m = g->max();
        if (m > res) {
            res = m;
        }
    }
    return res;
}

std::string Random::toString()
{
    if (generators.empty()) {
        return "";
    }
    std::uniform_int_distribution<std::size_t> dist(0, generators.size() - 1);
    return generators[dist(rng)]->toString();
}

Sequence::Sequence(std::vector<std::unique_ptr<NameGenerator>>&& generators_)
    : NameGenerator(std::move(generators_))
{
}

Literal::Literal(const std::string& value_)
    : value(value_)
{
}

std::size_t Literal::combinations()
{
    return 1;
}

std::size_t Literal::min()
{
    return value.size();
}

std::size_t Literal::max()
{
    return value.size();
}

std::string Literal::toString()
{
    return value;
}

Reverser::Reverser(std::unique_ptr<NameGenerator>&& g)
{
    add(std::move(g));
}

std::string Reverser::toString()
{
    std::wstring ws = towstring(NameGenerator::toString());
    std::reverse(ws.begin(), ws.end());
    return tostring(ws);
}

Capitalizer::Capitalizer(std::unique_ptr<NameGenerator>&& g)
{
    add(std::move(g));
}

std::string Capitalizer::toString()
{
    std::wstring ws = towstring(NameGenerator::toString());
    if (!ws.empty()) {
        ws[0] = std::towupper(ws[0]);
    }
    return tostring(ws);
}

Collapser::Collapser(std::unique_ptr<NameGenerator>&& g)
{
    add(std::move(g));
}

std::string Collapser::toString()
{
    std::wstring ws = towstring(NameGenerator::toString());
    std::wstring result;
    result.reserve(ws.size());

    int count = 0;
    wchar_t prev = L'\0';
    for (auto ch : ws) {
        if (ch == prev) {
            ++count;
        } else {
            count = 0;
        }
        // Default allow up to 2 repeats
        int maxRepeat = 2;

        // If you want to restrict certain letters to 1 repeat, etc.
        // For demonstration, keep it simple and always use maxRepeat=2
        // or adapt logic as needed.

        if (count < maxRepeat) {
            result.push_back(ch);
        }
        prev = ch;
    }

    return tostring(result);
}

}
