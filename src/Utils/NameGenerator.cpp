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
    // Exactly as in the original code, plus a new "X" key for "Big head
    // monster"
    static auto* const symbols = new std::unordered_map<std::string,
        const std::vector<std::string>>(
        { { "s",
              { "ach", "ack", "ad", "age", "ald", "ale", "an", "ang", "ar",
                  "ard", "as", "ash", "at", "ath", "augh", "aw", "ban", "bel",
                  "bur", "cer", "cha", "che", "dan", "dar", "del", "den", "dra",
                  "dyn", "ech", "eld", "elm", "em", "en", "end", "eng", "enth",
                  "er", "ess", "est", "et", "gar", "gha", "hat", "hin", "hon",
                  "ia", "ight", "ild", "im", "ina", "ine", "ing", "ir", "is",
                  "iss", "it", "kal", "kel", "kim", "kin", "ler", "lor", "lye",
                  "mor", "mos", "nal", "ny", "nys", "old", "om", "on", "or",
                  "orm", "os", "ough", "per", "pol", "qua", "que", "rad", "rak",
                  "ran", "ray", "ril", "ris", "rod", "roth", "ryn", "sam",
                  "say", "ser", "shy", "skel", "sul", "tai", "tan", "tas",
                  "ther", "tia", "tin", "ton", "tor", "tur", "um", "und", "unt",
                  "urn", "usk", "ust", "ver", "ves", "vor", "war", "wor",
                  "yer" } },
            { "v", { "a", "e", "i", "o", "u", "y" } },
            { "V",
                { "a", "e", "i", "o", "u", "y", "ae", "ai", "au", "ay", "ea",
                    "ee", "ei", "eu", "ey", "ia", "ie", "oe", "oi", "oo", "ou",
                    "ui" } },
            { "c",
                { "b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p",
                    "q", "r", "s", "t", "v", "w", "x", "y", "z" } },
            { "B",
                { "b", "bl", "br", "c", "ch", "chr", "cl", "cr", "d", "dr", "f",
                    "g", "h", "j", "k", "l", "ll", "m", "n", "p", "ph", "qu",
                    "r", "rh", "s", "sch", "sh", "sl", "sm", "sn", "st", "str",
                    "sw", "t", "th", "thr", "tr", "v", "w", "wh", "y", "z",
                    "zh" } },
            { "C",
                { "b", "c", "ch", "ck", "d", "f", "g", "gh", "h", "k", "l",
                    "ld", "ll", "lt", "m", "n", "nd", "nn", "nt", "p", "ph",
                    "q", "r", "rd", "rr", "rt", "s", "sh", "ss", "st", "t",
                    "th", "v", "w", "y", "z" } },
            { "i",
                { "air", "ankle", "ball", "beef", "bone", "bum", "bumble",
                    "bump", "cheese", "clod", "clot", "clown", "corn", "dip",
                    "dolt", "doof", "dork", "dumb", "face", "finger", "foot",
                    "fumble", "goof", "grumble", "head", "knock", "knocker",
                    "knuckle", "loaf", "lump", "lunk", "meat", "muck", "munch",
                    "nit", "numb", "pin", "puff", "skull", "snark", "sneeze",
                    "thimble", "twerp", "twit", "wad", "wimp", "wipe" } },
            { "m",
                { "baby", "booble", "bunker", "cuddle", "cuddly", "cutie",
                    "doodle", "foofie", "gooble", "honey", "kissie", "lover",
                    "lovey", "moofie", "mooglie", "moopie", "moopsie", "nookum",
                    "poochie", "poof", "poofie", "pookie", "schmoopie",
                    "schnoogle", "schnookie", "schnookum", "smooch", "smoochie",
                    "smoosh", "snoogle", "snoogy", "snookie", "snookum",
                    "snuggy", "sweetie", "woogle", "woogy", "wookie", "wookum",
                    "wuddle", "wuddly", "wuggy", "wunny" } },
            { "M",
                { "boo", "bunch", "bunny", "cake", "cakes", "cute", "darling",
                    "dumpling", "dumplings", "face", "foof", "goo", "head",
                    "kin", "kins", "lips", "love", "mush", "pie", "poo", "pooh",
                    "pook", "pums" } },
            { "D",
                { "b", "bl", "br", "cl", "d", "f", "fl", "fr", "g", "gh", "gl",
                    "gr", "h", "j", "k", "kl", "m", "n", "p", "th", "w" } },
            { "d",
                { "elch", "idiot", "ob", "og", "ok", "olph", "olt", "omph",
                    "ong", "onk", "oo", "oob", "oof", "oog", "ook", "ooz",
                    "org", "ork", "orm", "oron", "ub", "uck", "ug", "ulf",
                    "ult", "um", "umb", "ump", "umph", "un", "unb", "ung",
                    "unk", "unph", "unt", "uzz" } },

            // NEW: Symbol "X" to produce "Big head monster"-like strings
            { "X",
                { "Big head monster", "Tiny foot beast", "Giant claw fiend",
                    "Little bug goblin", "Huge horn demon",
                    "Dark wing creature", "Floating eye horror",
                    "Slime king mutant", "Deep sea crawler",
                    "Steel shell titan", "Shadow maw terror",
                    "Bone cruncher brute", "Venomous fang abomination",
                    "Rusted gear colossus", "Cyclopean nightmare",
                    "Warped flesh beast", "Electric doom juggernaut",
                    "Twisted limb horror", "Phantom shriek banshee",
                    "Molten core gargoyle", "Voidwalker aberration",
                    "Chitinous overlord", "Toxic spore tyrant",
                    "Bladed tentacle wraith", "Necrotic bone horror",
                    "Gigantic doom slug", "Howling void specter",
                    "Fungal hive monstrosity", "Titanic iron golem",
                    "Bio-mechanical hydra", "Screaming skull revenant",
                    "Crimson fang stalker", "Shattered soul construct",
                    "Warpstorm entity", "Eldritch horror spawn",
                    "Mutant broodmother", "Pulsating eyeball fiend",
                    "Plagueborn behemoth", "Cosmic rift fiend",
                    "Thunderous abyss tyrant", "Clockwork abomination",
                    "Dread maw leviathan", "Neon cyber revenant",
                    "Ravenous void beast", "Exo-skeletal nightmare",
                    "Demonic symbiote horror", "Omega flesh colossus",
                    "Starborn parasite", "Titanic fungal overlord",
                    "Infernal core berserker", "Venomous shadow slasher",
                    "Neural web terror", "Glass-jawed titan",
                    "Hyper-mutant anomaly", "Psionic rift horror",
                    "Ebon fang predator", "Magma blood behemoth",
                    "Giga-eyed watcher", "Sporecloud phantom",
                    "Undying doom knight", "Shapeless void horror",
                    "Radioactive fiend", "Obsidian armored leviathan",
                    "Lurking mind eater", "Pale mask fiend",
                    "Chain-claw butcher", "Warped metallic ogre",
                    "Neon-lit terror hound", "Acid-flesh crawler",
                    "Spiked husk revenant", "Unstable energy wraith",
                    "Galactic rift anomaly", "Shadowstalker lich",
                    "Hyper-adaptive chimera", "Gilded bone nightmare",
                    "Quantum dread titan", "Aetherbound specter",
                    "Molten iron horror", "Gore-claw ghoul",
                    "Vortex-born phantom", "Dagger-jawed gargoyle",
                    "Clockwork hive mind", "Glass-eye trickster",
                    "Plasma-veined reaper", "Feral steel berserker",
                    "Howling spectral fiend", "Voidborn flesh weaver",
                    "Eternal storm behemoth", "Neon circuitry revenant",
                    "Blood-soaked warlord", "Frozen nightmare predator",
                    "Thousand-eyed horror", "Oblivion-strider monstrosity",
                    "Corrupted war automaton", "Thunder-charged titan",
                    "Cybernetic necro-sentinel", "Titanic mind parasite",
                    "Abyssal rune-walker", "Shadow-marked terror fiend",
                    "Bio-luminescent nightmare", "Inverted flesh golem",
                    "Silent screaming horror",
                    "Warped skeletal juggernaut" } } });
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
