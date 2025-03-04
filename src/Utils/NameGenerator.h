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

#ifndef NAMEGENERATOR_H
#define NAMEGENERATOR_H

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cwchar>
#include <cwctype>
#include <memory>
#include <random>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Daitengu::Utils {

std::wstring towstring(const std::string& s);
std::string tostring(const std::wstring& ws);

class NameGenerator {
public:
    NameGenerator();
    NameGenerator(const std::string& pattern, bool collapse_triples = true);
    NameGenerator(std::vector<std::unique_ptr<NameGenerator>>&& generators_);

    virtual ~NameGenerator() = default;

    virtual std::size_t combinations();
    virtual std::size_t min();
    virtual std::size_t max();
    virtual std::string toString();
    void add(std::unique_ptr<NameGenerator>&& g);

protected:
    std::vector<std::unique_ptr<NameGenerator>> generators;

private:
    enum class wrappers { capitalizer, reverser };

    enum class group_types { symbol, literal };

    class Group {
    public:
        group_types type;
        Group(group_types t);
        virtual ~Group() = default;

        std::unique_ptr<NameGenerator> produce();
        void split();
        void wrap(wrappers w);
        void add(std::unique_ptr<NameGenerator>&& g);
        virtual void add(char c);

    protected:
        std::stack<wrappers> wrapperStack;
        std::vector<std::unique_ptr<NameGenerator>> set;
    };

    class GroupSymbol : public Group {
    public:
        GroupSymbol();
        void add(char c) override; // override to expand symbol from SymbolMap
    };

    class GroupLiteral : public Group {
    public:
        GroupLiteral();
    };

    static const std::unordered_map<std::string,
        const std::vector<std::string>>&
    SymbolMap();
};

class Random : public NameGenerator {
public:
    Random() = default;
    explicit Random(std::vector<std::unique_ptr<NameGenerator>>&& generators_);

    std::size_t combinations() override;
    std::size_t min() override;
    std::size_t max() override;
    std::string toString() override;
};

class Sequence : public NameGenerator {
public:
    Sequence() = default;
    explicit Sequence(
        std::vector<std::unique_ptr<NameGenerator>>&& generators_);
};

class Literal : public NameGenerator {
public:
    explicit Literal(const std::string& value_);

    std::size_t combinations() override;
    std::size_t min() override;
    std::size_t max() override;
    std::string toString() override;

private:
    std::string value;
};

class Reverser : public NameGenerator {
public:
    explicit Reverser(std::unique_ptr<NameGenerator>&& g);
    std::string toString() override;
};

class Capitalizer : public NameGenerator {
public:
    explicit Capitalizer(std::unique_ptr<NameGenerator>&& g);
    std::string toString() override;
};

class Collapser : public NameGenerator {
public:
    explicit Collapser(std::unique_ptr<NameGenerator>&& g);
    std::string toString() override;
};

inline constexpr auto MIDDLE_EARTH
    = "(bil|bal|ban|hil|ham|hal|hol|hob|wil|me|or|ol|od|gor|for|fos|tol|ar|fin|"
      "ere|leo|vi|bi|bren|thor)"
      "(|go|orbis|apol|adur|mos|ri|i|na|ole|n)"
      "(|tur|axia|and|bo|gil|bin|bras|las|mac|grim|wise|l|lo|fo|co|ra|via|da|"
      "ne|ta|y|wen|thiel|phin|dir|dor|tor|rod|on|rdo|dis)";

inline constexpr auto JAPANESE_NAMES_CONSTRAINED
    = "(aka|aki|bashi|gawa|kawa|furu|fuku|fuji|hana|hara|haru|hashi|hira|hon|"
      "hoshi|ichi|iwa|kami|kawa|ki|kita|kuchi|kuro|marui|matsu|miya|mori|moto|"
      "mura|nabe|naka|nishi|no|da|ta|o|oo|oka|saka|saki|sawa|shita|shima|i|"
      "suzu|taka|take|to|toku|toyo|ue|wa|wara|wata|yama|yoshi|kei|ko|zawa|zen|"
      "sen|ao|gin|kin|ken|shiro|zaki|yuki|asa)"
      "(||||||||||bashi|gawa|kawa|furu|fuku|fuji|hana|hara|haru|hashi|hira|hon|"
      "hoshi|chi|wa|ka|kami|kawa|ki|kita|kuchi|kuro|marui|matsu|miya|mori|moto|"
      "mura|nabe|naka|nishi|no|da|ta|o|oo|oka|saka|saki|sawa|shita|shima|suzu|"
      "taka|take|to|toku|toyo|ue|wa|wara|wata|yama|yoshi|kei|ko|zawa|zen|sen|"
      "ao|gin|kin|ken|shiro|zaki|yuki|sa)";

inline constexpr auto JAPANESE_NAMES_DIVERSE
    = "(a|i|u|e|o|||||)(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|"
      "se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|"
      "ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|"
      "ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)"
      "(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|"
      "chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|"
      "mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|"
      "ro|wa|wa|wa|wa|wo|wo)"
      "(|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|"
      "chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|"
      "mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|"
      "ro|ro|wa|wa|wa|wa|wo|wo)"
      "|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|"
      "chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|"
      "mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|"
      "ro|wa|wa|wa|wa|wo|wo)"
      "(|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|"
      "chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|"
      "mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|"
      "ro|ro|wa|wa|wa|wa|wo|wo)))(|||n)";

inline constexpr auto CHINESE_NAMES
    = "(zh|x|q|sh|h)(ao|ian|uo|ou|ia)"
      "(|(l|w|c|p|b|m)(ao|ian|uo|ou|ia)(|n)|-(l|w|c|p|b|m)(ao|ian|uo|ou|ia)(|("
      "d|j|q|l)(a|ai|iu|ao|i)))";

inline constexpr auto GREEK_NAMES
    = "<s<v|V>(tia)|s<v|V>(os)|B<v|V>c(ios)|B<v|V><c|C>v(ios|os)>";

inline constexpr auto HAWAIIAN_NAMES_1
    = "((h|k|l|m|n|p|w|')|)(a|e|i|o|u)((h|k|l|m|n|p|w|')|)(a|e|i|o|u)"
      "(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)"
      "(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)";

inline constexpr auto HAWAIIAN_NAMES_2
    = "((h|k|l|m|n|p|w|)(a|e|i|o|u|a'|e'|i'|o'|u'|ae|ai|ao|au|oi|ou|eu|ei)(k|l|"
      "m|n|p|)|)"
      "(h|k|l|m|n|p|w|)(a|e|i|o|u|a'|e'|i'|o'|u'|ae|ai|ao|au|oi|ou|eu|ei)(k|l|"
      "m|n|p|)";

inline constexpr auto OLD_LATIN_PLACE_NAMES = "sv(nia|lia|cia|sia)";

inline constexpr auto DRAGONS_PERN
    = "<<s|ss>|<VC|vC|B|BVs|Vs>><v|V|v|<v(l|n|r)|vc>>(th)";

inline constexpr auto DRAGON_RIDERS = "c'<s|cvc>";

inline constexpr auto POKEMON = "<i|s>v(mon|chu|zard|rtle)";

inline constexpr auto FANTASY_VOWELS_R
    = "(|(<B>|s|h|ty|ph|r))(i|ae|ya|ae|eu|ia|i|eo|ai|a)"
      "(lo|la|sri|da|dai|the|sty|lae|due|li|lly|ri|na|ral|sur|rith)"
      "(|(su|nu|sti|llo|ria|))(|(n|ra|p|m|lis|cal|deu|dil|suir|phos|ru|dru|rin|"
      "raap|rgue))";

inline constexpr auto FANTASY_S_A
    = "(cham|chan|jisk|lis|frich|isk|lass|mind|sond|sund|ass|chad|lirt|und|mar|"
      "lis|il|<BVC>)"
      "(jask|ast|ista|adar|irra|im|ossa|assa|osia|ilsa|<vCv>)"
      "(|(an|ya|la|sta|sda|sya|st|nya))";

inline constexpr auto FANTASY_H_L
    = "(ch|ch't|sh|cal|val|ell|har|shar|shal|rel|laen|ral|jh't|alr|ch|ch't|av)"
      "(|(is|al|ow|ish|ul|el|ar|iel))"
      "(aren|aeish|aith|even|adur|ulash|alith|atar|aia|erin|aera|ael|ira|iel|"
      "ahur|ishul)";

inline constexpr auto FANTASY_N_L
    = "(ethr|qil|mal|er|eal|far|fil|fir|ing|ind|il|lam|quel|quar|quan|qar|pal|"
      "mal|yar|um|ard|enn|ey)"
      "(|(<vc>|on|us|un|ar|as|en|ir|ur|at|ol|al|an))"
      "(uard|wen|arn|on|il|ie|on|iel|rion|rian|an|ista|rion|rian|cil|mol|yon)";

inline constexpr auto FANTASY_K_N
    = "(taith|kach|chak|kank|kjar|rak|kan|kaj|tach|rskal|kjol|jok|jor|jad|kot|"
      "kon|knir|kror|kol|tul|rhaok|rhak|krol|jan|kag|ryr)"
      "(<vc>|in|or|an|ar|och|un|mar|yk|ja|arn|ir|ros|ror)"
      "(|(mund|ard|arn|karr|chim|kos|rir|arl|kni|var|an|in|ir|a|i|as))";

inline constexpr auto FANTASY_J_G_Z
    = "(aj|ch|etz|etzl|tz|kal|gahn|kab|aj|izl|ts|jaj|lan|kach|chaj|qaq|jol|ix|"
      "az|biq|nam)"
      "(|(<vc>|aw|al|yes|il|ay|en|tom||oj|im|ol|aj|an|as))"
      "(aj|am|al|aqa|ende|elja|ich|ak|ix|in|ak|al|il|ek|ij|os|al|im)";

inline constexpr auto FANTASY_K_J_Y
    = "(yi|shu|a|be|na|chi|cha|cho|ksa|yi|shu)(th|dd|jj|sh|rr|mk|n|rk|y|jj|th)"
      "(us|ash|eni|akra|nai|ral|ect|are|el|urru|aja|al|uz|ict|arja|ichi|ural|"
      "iru|aki|esh)";

inline constexpr auto FANTASY_S_E
    = "(syth|sith|srr|sen|yth|ssen|then|fen|ssth|kel|syn|est|bess|inth|nen|tin|"
      "cor|sv|iss|ith|sen|slar|ssil|sthen|svis|s|ss|s|ss)"
      "(|(tys|eus|yn|of|es|en|ath|elth|al|ell|ka|ith|yrrl|is|isl|yr|ast|iy))"
      "(us|yn|en|ens|ra|rg|le|en|ith|ast|zon|in|yn|ys)";

}
#endif // NAMEGENERATOR_H
