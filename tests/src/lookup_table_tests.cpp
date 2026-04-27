#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "lookup_table/LookUpTable.h"

namespace {
const std::string SHAKESPEARE_KEY_1 =
    "To be, or not to be, that is the question: Whether 'tis nobler in the mind";
const std::string SHAKESPEARE_VAL_1 =
    "To die, to sleep—No more; and by a sleep to say we end the heart-ache and the thousand natural shocks";

const std::string SHAKESPEARE_KEY_2 =
    "And by opposing end them. To die—to sleep, No more; and by a sleep to say we end the heart's woe";
const std::string SHAKESPEARE_VAL_2 =
    "The slings and arrows of outrageous fortune, Or to take arms against a sea of troubles and lose";

const std::string SHAKESPEARE_KEY_3 =
    "All the world's a stage, And all the men and women merely players with their own scripts";
const std::string SHAKESPEARE_VAL_3 =
    "They have their exits and their entrances, And one man in his time plays many parts upon this mortal stage";

const std::string SHAKESPEARE_KEY_4 =
    "Out, out, brief candle! Life's but a walking shadow, a poor player that struts and frets his hour";
const std::string SHAKESPEARE_VAL_4 =
    "It is a tale told by an idiot, full of sound and fury, signifying nothing in the end for all our struggles";

const std::string SHAKESPEARE_KEY_5 =
    "To sleep, perchance to dream—ay, there's the rub, For in that sleep of death what dreams may come after";
const std::string SHAKESPEARE_VAL_5 =
    "The whips and scorns of time, the oppressor's wrong, the pangs of despised love, the law's delay";
}

TEST_CASE("LookUpTable: insert/find/erase", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    REQUIRE(t.empty());

    t.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    t.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});
    REQUIRE(t.size() == 2);

    REQUIRE(t.find(SHAKESPEARE_KEY_1) != t.end());
    REQUIRE(t.at(SHAKESPEARE_KEY_2) == SHAKESPEARE_VAL_2);

    t[SHAKESPEARE_KEY_3] = SHAKESPEARE_VAL_3;
    REQUIRE(t.size() == 3);
    REQUIRE(t.at(SHAKESPEARE_KEY_3) == SHAKESPEARE_VAL_3);

    REQUIRE(t.erase(SHAKESPEARE_KEY_2) == 1);
    REQUIRE(t.count(SHAKESPEARE_KEY_2) == 0);
}

TEST_CASE("LookUpTable: constructors/get_allocator/max_size", "[lookup_table]") {
    LookUpTable<std::string, std::string> t1(8);
    REQUIRE(t1.empty());
    REQUIRE(t1.size() == 0);
    REQUIRE(t1.capacity() == 8);

    LookUpTable<std::string, std::string> t2(std::allocator<std::pair<const std::string, std::string>>{});
    (void)t2.get_allocator();

    REQUIRE(t2.max_size() > 0);
}

TEST_CASE("LookUpTable: range/init_list ctors + init_list operator=", "[lookup_table]") {
    std::vector<std::pair<std::string, std::string>> src{
        {SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1}, 
        {SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2}
    };
    LookUpTable<std::string, std::string> t1(src.begin(), src.end());
    REQUIRE(t1.size() == 2);
    REQUIRE(t1.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
    REQUIRE(t1.at(SHAKESPEARE_KEY_2) == SHAKESPEARE_VAL_2);

    LookUpTable<std::string, std::string> t2{{
        {SHAKESPEARE_KEY_3, SHAKESPEARE_VAL_3}, 
        {SHAKESPEARE_KEY_4, SHAKESPEARE_VAL_4}
    }};
    REQUIRE(t2.size() == 2);
    REQUIRE(t2.at(SHAKESPEARE_KEY_3) == SHAKESPEARE_VAL_3);

    t2 = {{SHAKESPEARE_KEY_5, SHAKESPEARE_VAL_5}};
    REQUIRE(t2.size() == 1);
    REQUIRE(t2.at(SHAKESPEARE_KEY_5) == SHAKESPEARE_VAL_5);
}

TEST_CASE("LookUpTable: итераторы", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    t.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    t.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});

    int count = 0;
    for (auto it = t.begin(); it != t.end(); ++it) {
        REQUIRE(it->first.size() > 64);
        REQUIRE(it->second.size() > 64);
        count++;
    }
    REQUIRE(count == 2);
}

TEST_CASE("LookUpTableIterator: operator-> / -- / postfix", "[lookup_table][iterator]") {
    LookUpTable<std::string, std::string> t;
    t.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    t.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});
    t.insert({SHAKESPEARE_KEY_3, SHAKESPEARE_VAL_3});

    auto it = t.begin();
    REQUIRE(it->first == SHAKESPEARE_KEY_1);
    REQUIRE(it->second == SHAKESPEARE_VAL_1);

    auto endIt = t.end();
    --endIt;
    auto lastKey = endIt->first;
    auto lastVal = endIt->second;
    REQUIRE(lastKey.size() > 64);
    REQUIRE(lastVal.size() > 64);

    auto post = endIt--;
    REQUIRE(post->first == lastKey);
    REQUIRE(post->second == lastVal);
    REQUIRE(endIt != t.end());
    REQUIRE(endIt->first != lastKey);
}

TEST_CASE("LookUpTable: at() throws + const find/at", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    t.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});

    REQUIRE_THROWS_AS(t.at(SHAKESPEARE_KEY_2), std::out_of_range);

    const auto& ct = t;
    REQUIRE(ct.find(SHAKESPEARE_KEY_1) != ct.end());
    REQUIRE(ct.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
    REQUIRE_THROWS_AS(ct.at(SHAKESPEARE_KEY_2), std::out_of_range);
}

TEST_CASE("LookUpTable: insert rvalue + emplace (no overwrite on existing key)", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;

    std::pair<const std::string, std::string> p{SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1};
    auto [it1, inserted1] = t.insert(std::move(p));
    REQUIRE(inserted1);
    REQUIRE(it1->first == SHAKESPEARE_KEY_1);
    REQUIRE(it1->second == SHAKESPEARE_VAL_1);

    auto [it2, inserted2] = t.emplace(SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_2);
    REQUIRE_FALSE(inserted2);
    REQUIRE(it2->first == SHAKESPEARE_KEY_1);
    REQUIRE(it2->second == SHAKESPEARE_VAL_1);
    REQUIRE(t.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
}

TEST_CASE("LookUpTable: copy/move/swap + сравнение", "[lookup_table]") {
    LookUpTable<std::string, std::string> a;
    a.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    a.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});

    LookUpTable<std::string, std::string> b(a);
    REQUIRE(b == a);

    LookUpTable<std::string, std::string> c;
    c = a;
    REQUIRE(c == a);

    LookUpTable<std::string, std::string> moved(std::move(c));
    REQUIRE(moved.size() == 2);
    REQUIRE(moved.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
    REQUIRE(moved.at(SHAKESPEARE_KEY_2) == SHAKESPEARE_VAL_2);

    LookUpTable<std::string, std::string> d;
    d.insert({SHAKESPEARE_KEY_3, SHAKESPEARE_VAL_3});
    d.swap(moved);
    REQUIRE(d.count(SHAKESPEARE_KEY_1) == 1);
    REQUIRE(moved.count(SHAKESPEARE_KEY_3) == 1);

    swap(d, moved);
    REQUIRE(d.count(SHAKESPEARE_KEY_3) == 1);
}

TEST_CASE("LookUpTable: move assignment", "[lookup_table]") {
    LookUpTable<std::string, std::string> src;
    src.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    src.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});

    LookUpTable<std::string, std::string> dst;
    dst = std::move(src);
    REQUIRE(dst.size() == 2);
    REQUIRE(dst.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
    REQUIRE(dst.at(SHAKESPEARE_KEY_2) == SHAKESPEARE_VAL_2);
}

TEST_CASE("LookUpTable: assign/clear/reserve/capacity", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;

    std::vector<std::pair<const std::string, std::string>> v{
        {SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1}, 
        {SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2}, 
        {SHAKESPEARE_KEY_3, SHAKESPEARE_VAL_3}
    };
    t.assign(v.begin(), v.end());
    REQUIRE(t.size() == 3);

    t.assign({{SHAKESPEARE_KEY_4, SHAKESPEARE_VAL_4}});
    REQUIRE(t.size() == 1);
    REQUIRE(t.at(SHAKESPEARE_KEY_4) == SHAKESPEARE_VAL_4);

    t.clear();
    REQUIRE(t.empty());

    t.insert({SHAKESPEARE_KEY_5, SHAKESPEARE_VAL_5});
    t.reserve(16);
    REQUIRE(t.capacity() >= 16);
    REQUIRE(t.at(SHAKESPEARE_KEY_5) == SHAKESPEARE_VAL_5);
}

TEST_CASE("LookUpTable: erase(iterator) + erase(range)", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    t.insert({SHAKESPEARE_KEY_1, SHAKESPEARE_VAL_1});
    t.insert({SHAKESPEARE_KEY_2, SHAKESPEARE_VAL_2});
    t.insert({SHAKESPEARE_KEY_3, SHAKESPEARE_VAL_3});

    auto next = t.find(SHAKESPEARE_KEY_2);
    auto erased = t.erase(next);
    REQUIRE(t.count(SHAKESPEARE_KEY_2) == 0);
    REQUIRE((erased == t.end() || erased->first == SHAKESPEARE_KEY_3));
    t.erase(t.begin(), t.end());
    REQUIRE(t.empty());
}

TEST_CASE("LookUpTable: erase(end()) is no-op", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    REQUIRE(t.erase(t.end()) == t.end());
}

TEST_CASE("LookUpTable: operator[] rvalue overload", "[lookup_table]") {
    LookUpTable<std::string, std::string> t;
    t[std::string(SHAKESPEARE_KEY_1)] = SHAKESPEARE_VAL_1;
    REQUIRE(t.at(SHAKESPEARE_KEY_1) == SHAKESPEARE_VAL_1);
}

TEST_CASE("LookUpTable: long string keys/values (>64 chars)", "[lookup_table][strings]") {
    LookUpTable<std::string, std::string> t;

    const std::string key1 =
        "To be, or not to be, that is the question: Whether 'tis nobler in the mind";
    const std::string val1 =
        "To die, to sleep—No more; and by a sleep to say we end the heart-ache";

    const std::string key2 =
        "And by opposing end them. To die—to sleep, No more; and by a sleep to say we end";
    const std::string val2 =
        "The slings and arrows of outrageous fortune, Or to take arms against a sea of troubles";

    REQUIRE(key1.size() > 64);
    REQUIRE(val1.size() > 64);
    REQUIRE(key2.size() > 64);
    REQUIRE(val2.size() > 64);

    auto [it1, inserted1] = t.insert({key1, val1});
    REQUIRE(inserted1);
    REQUIRE(it1 != t.end());

    auto [it2, inserted2] = t.insert({key2, val2});
    REQUIRE(inserted2);
    REQUIRE(it2 != t.end());

    REQUIRE(t.size() == 2);
    REQUIRE(t.at(key1) == val1);
    REQUIRE(t.at(key2) == val2);

    const std::string val1_updated =
        val1 + " and the thousand natural shocks that flesh is heir to; 'tis a consummation.";
    REQUIRE(val1_updated.size() > 64);
    t[key1] = val1_updated;
    REQUIRE(t.at(key1) == val1_updated);

    REQUIRE(t.erase(key2) == 1);
    REQUIRE(t.count(key2) == 0);
}
