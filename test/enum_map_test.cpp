#include "fixed_containers/enum_map.hpp"

#include "enums_test_common.hpp"
#include "mock_testing_types.hpp"

#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <iterator>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
namespace
{
using ES_1 = EnumMap<TestEnum1, int>;
using ES_2 = EnumMap<TestRichEnum1, int>;
using ES_3 = EnumMap<NonConformingTestRichEnum1, int>;

static_assert(std::is_trivially_copyable_v<ES_1>);
static_assert(!std::is_trivial_v<ES_1>);
static_assert(std::is_standard_layout_v<ES_1>);
static_assert(std::is_trivially_copy_assignable_v<ES_1>);
static_assert(std::is_trivially_move_assignable_v<ES_1>);

static_assert(std::is_trivially_copyable_v<ES_2>);
static_assert(!std::is_trivial_v<ES_2>);
static_assert(std::is_standard_layout_v<ES_2>);
static_assert(std::is_trivially_copy_assignable_v<ES_2>);
static_assert(std::is_trivially_move_assignable_v<ES_2>);

static_assert(std::is_trivially_copyable_v<ES_3>);
static_assert(!std::is_trivial_v<ES_3>);
static_assert(std::is_standard_layout_v<ES_3>);
static_assert(std::is_trivially_copy_assignable_v<ES_3>);
static_assert(std::is_trivially_move_assignable_v<ES_3>);

static_assert(ranges::bidirectional_iterator<ES_1::iterator>);
static_assert(ranges::bidirectional_iterator<ES_1::const_iterator>);
}  // namespace

TEST(Utilities, EnumMap_DefaultCtor)
{
    constexpr EnumMap<TestEnum1, int> s1{};
    static_assert(s1.empty());
}

TEST(Utilities, EnumMap_Initializer)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::THREE, 30}};
    static_assert(s2.size() == 1);
}

TEST(Utilities, EnumMap_Builder_FluentSyntaxWithNoExtraCopies)
{
    constexpr std::array<std::pair<TestRichEnum1, int>, 2> a{
        std::pair{TestRichEnum1::C_THREE(), 33},
        std::pair{TestRichEnum1::C_THREE(), 33},
    };
    constexpr std::pair<TestRichEnum1, int> b = {TestRichEnum1::C_TWO(), 22};

    constexpr auto s1 = EnumMap<TestRichEnum1, int>::Builder{}
                            .insert(b)
                            .insert({TestRichEnum1::C_TWO(), 22222})
                            .insert({
                                {TestRichEnum1::C_THREE(), 33},
                                {TestRichEnum1::C_FOUR(), 44},
                            })
                            .insert(a.cbegin(), a.cend())
                            .build();

    static_assert(s1.size() == 3);

    static_assert(!s1.contains(TestRichEnum1::C_ONE()));
    static_assert(s1.contains(TestRichEnum1::C_TWO()));
    static_assert(s1.contains(TestRichEnum1::C_THREE()));
    static_assert(s1.contains(TestRichEnum1::C_FOUR()));

    static_assert(s1.at(TestRichEnum1::C_TWO()) == 22);  // First value inserted wins
    static_assert(s1.at(TestRichEnum1::C_THREE()) == 33);
    static_assert(s1.at(TestRichEnum1::C_FOUR()) == 44);
}

TEST(Utilities, EnumMap_Builder_MultipleOuts)
{
    constexpr std::array<std::pair<TestEnum1, int>, 2> a{
        std::pair{TestEnum1::THREE, 33},
        std::pair{TestEnum1::THREE, 33},
    };
    constexpr std::pair<TestEnum1, int> b = {TestEnum1::TWO, 22};

    constexpr std::array<EnumMap<TestEnum1, int>, 2> s_all = [&]()
    {
        EnumMap<TestEnum1, int>::Builder builder{};

        builder.insert(b);
        auto out1 = builder.build();

        // l-value overloads
        builder.insert(a.begin(), a.end());
        builder.insert(b);
        builder.insert({TestEnum1::TWO, 22222});
        builder.insert({{TestEnum1::THREE, 33}, {TestEnum1::FOUR, 44}});
        auto out2 = builder.build();

        return std::array<EnumMap<TestEnum1, int>, 2>{out1, out2};
    }();

    {
        // out1 should be unaffected by out2's addition of extra elements
        constexpr EnumMap<TestEnum1, int> s1 = s_all[0];
        static_assert(s1.size() == 1);

        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));

        static_assert(s1.at(TestEnum1::TWO) == 22);
    }

    {
        constexpr EnumMap<TestEnum1, int> s2 = s_all[1];
        static_assert(s2.size() == 3);

        static_assert(!s2.contains(TestEnum1::ONE));
        static_assert(s2.contains(TestEnum1::TWO));
        static_assert(s2.contains(TestEnum1::THREE));
        static_assert(s2.contains(TestEnum1::FOUR));

        static_assert(s2.at(TestEnum1::TWO) == 22);  // First value inserted wins
        static_assert(s2.at(TestEnum1::THREE) == 33);
        static_assert(s2.at(TestEnum1::FOUR) == 44);
    }
}

TEST(Utilities, EnumMap_StaticFactory_CreateWithKeys)
{
    constexpr FixedVector<TestEnum1, 5> keys = {TestEnum1 ::ONE, TestEnum1 ::FOUR};

    constexpr EnumMap<TestEnum1, int> s1 = EnumMap<TestEnum1, int>::create_with_keys(keys, -17);
    static_assert(s1.size() == 2);

    static_assert(s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));

    static_assert(s1.at(TestEnum1::ONE) == -17);
    static_assert(s1.at(TestEnum1::FOUR) == -17);
}

TEST(Utilities, EnumMap_CreateWithAllEntries)
{
    constexpr auto s1 = EnumMap<TestEnum1, int>::create_with_all_entries({
        {TestEnum1::ONE, 42},
        {TestEnum1::TWO, 7},
        {TestEnum1::THREE, 42},
        {TestEnum1::FOUR, 7},
    });

    static_assert(s1.size() == 4);
    static_assert(s1.at(TestEnum1::ONE) == 42);
    static_assert(s1.at(TestEnum1::TWO) == 7);
    static_assert(s1.at(TestEnum1::THREE) == 42);
    static_assert(s1.at(TestEnum1::FOUR) == 7);

    // must not compile:
    //    constexpr auto s2 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //        {TestEnum1::ONE, 42},
    //        {TestEnum1::THREE, 42},
    //        {TestEnum1::FOUR, 7},
    //    });
    //
    // however, if the following dies, that indicates assertion is working, and constexpr functions
    // that follow an assertion path will fail at compile time, with something like:
    //   > 's2' must be initialized by a constant expression
    //   >     constexpr auto s2 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //   >  '__assert_fail' cannot be used in a constant expression
    const auto get_incomplete_map = []
    {
        // can't put directly in macro due to curly braces for initializer list
        // return from lambda instead
        const auto s2 = EnumMap<TestEnum1, int>::create_with_all_entries({
            {TestEnum1::ONE, 42},
            {TestEnum1::THREE, 42},
            {TestEnum1::FOUR, 7},
        });
        return s2;
    };

    EXPECT_DEATH(get_incomplete_map(), "");
}

TEST(Utilities, EnumMap_EmptyAndSize)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr EnumMap<TestEnum1, int> s2{};
    static_assert(s2.empty());
}

TEST(Utilities, EnumMap_OperatorBracket_Constexpr)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s[TestEnum1::TWO] = 20;
        s[TestEnum1::FOUR] = 40;
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_OperatorBracket_NonConstexpr)
{
    EnumMap<TestEnum1, int> s1{};
    s1[TestEnum1::TWO] = 25;
    s1[TestEnum1::FOUR] = 45;
    ASSERT_EQ(2, s1.size());
    ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
    ASSERT_TRUE(s1.contains(TestEnum1::TWO));
    ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
    ASSERT_TRUE(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_Insert)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::TWO, 20});
        s.insert({TestEnum1::FOUR, 40});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        {
            auto [it, was_inserted] = s.insert({TestEnum1::TWO, 20});
            assert(was_inserted);
            assert(TestEnum1::TWO == it->first());
            assert(20 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::FOUR, 40});
            assert(was_inserted);
            assert(TestEnum1::FOUR == it->first());
            assert(40 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::TWO, 99999});
            assert(!was_inserted);
            assert(TestEnum1::TWO == it->first());
            assert(20 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::FOUR, 88888});
            assert(!was_inserted);
            assert(TestEnum1::FOUR == it->first());
            assert(40 == it->second());
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_InsertIterators)
{
    constexpr EnumMap<TestEnum1, int> a{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

    constexpr auto s1 = [&]()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_InsertInitializer)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_TryEmplace)
{
    EnumMap<TestEnum1, int> s1{};

    {
        auto [it, was_inserted] = s1.try_emplace(TestEnum1::TWO, 20);

        ASSERT_EQ(1, s1.size());
        ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
        ASSERT_TRUE(s1.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
        ASSERT_EQ(20, s1.at(TestEnum1::TWO));
        ASSERT_TRUE(was_inserted);
        ASSERT_EQ(TestEnum1::TWO, it->first());
        ASSERT_EQ(20, it->second());
    }

    {
        auto [it, was_inserted] = s1.try_emplace(TestEnum1::TWO, 209999999);
        ASSERT_EQ(1, s1.size());
        ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
        ASSERT_TRUE(s1.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
        ASSERT_EQ(20, s1.at(TestEnum1::TWO));
        ASSERT_FALSE(was_inserted);
        ASSERT_EQ(TestEnum1::TWO, it->first());
        ASSERT_EQ(20, it->second());
    }
}

TEST(Utilities, EnumMap_Emplace)
{
    EnumMap<TestEnum1, int> s1{};

    {
        auto [it, was_inserted] = s1.emplace(TestEnum1::TWO, 20);

        ASSERT_EQ(1, s1.size());
        ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
        ASSERT_TRUE(s1.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
        ASSERT_EQ(20, s1.at(TestEnum1::TWO));
        ASSERT_TRUE(was_inserted);
        ASSERT_EQ(TestEnum1::TWO, it->first());
        ASSERT_EQ(20, it->second());
    }

    {
        auto [it, was_inserted] = s1.emplace(TestEnum1::TWO, 209999999);
        ASSERT_EQ(1, s1.size());
        ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
        ASSERT_TRUE(s1.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
        ASSERT_EQ(20, s1.at(TestEnum1::TWO));
        ASSERT_FALSE(was_inserted);
        ASSERT_EQ(TestEnum1::TWO, it->first());
        ASSERT_EQ(20, it->second());
    }
}

TEST(Utilities, EnumMap_Clear)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(Utilities, EnumMap_Erase)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto removed_count = s.erase(TestEnum1::TWO);
        assert(removed_count == 1);
        removed_count = s.erase(TestEnum1::THREE);
        assert(removed_count == 0);
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumMap_Iterator_StructuredBinding)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::THREE, 30});
        s.insert({TestEnum1::FOUR, 40});
        s.insert({TestEnum1::ONE, 10});
        return s;
    }();

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }
}

TEST(Utilities, EnumMap_IteratorBasic)
{
    constexpr EnumMap<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(s1.begin()->first() == TestEnum1::ONE);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == TestEnum1::TWO);
    static_assert(std::next(s1.begin(), 1)->second() == 20);
    static_assert(std::next(s1.begin(), 2)->first() == TestEnum1::THREE);
    static_assert(std::next(s1.begin(), 2)->second() == 30);
    static_assert(std::next(s1.begin(), 3)->first() == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 3)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == TestEnum1::THREE);
    static_assert(std::prev(s1.end(), 2)->second() == 30);
    static_assert(std::prev(s1.end(), 3)->first() == TestEnum1::TWO);
    static_assert(std::prev(s1.end(), 3)->second() == 20);
    static_assert(std::prev(s1.end(), 4)->first() == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 4)->second() == 10);
}

TEST(Utilities, EnumMap_IteratorTypes)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        for (const auto& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const PairView<const TestEnum1, int>&>);
            // key_and_value.second() = 5; Not allowed
        }

        for (auto& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), PairView<const TestEnum1, int>&>);
            key_and_value.second() = 5;  // Allowed
        }

        for (auto&& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), PairView<const TestEnum1, int>&>);
            key_and_value.second() = 5;  // Allowed
        }

        for (const auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), const int&>);
        }

        for (auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return s;
    }();

    static_assert(
        std::is_same_v<decltype(*s1.begin()), const PairView<const TestEnum1, const int>&>);

    EnumMap<TestEnum1, int> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), PairView<const TestEnum1, int>&>);

    for (const auto& key_and_value : s1)
    {
        static_assert(
            std::is_same_v<decltype(key_and_value), const PairView<const TestEnum1, const int>&>);
    }

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::map<TestEnum1, int> std_map{};
        for (auto&& [key, v] : std_map)
        {
            (void)key;
            (void)v;
        }

        EnumMap<TestEnum1, int> this_map{};
        for (auto&& [key, v] : this_map)
        {
            (void)key;
            (void)v;
        }
    }
}

TEST(Utilities, EnumMap_IteratorMutableValue)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        for (auto&& [key, value] : s)
        {
            value *= 2;
        }

        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first() == TestEnum1::TWO);
    static_assert(s1.begin()->second() == 40);
    static_assert(std::next(s1.begin(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 1)->second() == 80);

    static_assert(std::prev(s1.end(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second() == 80);
    static_assert(std::prev(s1.end(), 2)->first() == TestEnum1::TWO);
    static_assert(std::prev(s1.end(), 2)->second() == 40);
}

TEST(Utilities, EnumMap_IteratorComparisonOperator)
{
    constexpr EnumMap<TestEnum1, int> s1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

    // All combinations of [==, !=]x[const, non-const]
    static_assert(s1.cbegin() == s1.cbegin());
    static_assert(s1.cbegin() == s1.begin());
    static_assert(s1.begin() == s1.begin());
    static_assert(s1.cbegin() != s1.cend());
    static_assert(s1.cbegin() != s1.end());
    static_assert(s1.begin() != s1.cend());

    static_assert(std::next(s1.begin(), 2) == s1.end());
    static_assert(std::prev(s1.end(), 2) == s1.begin());
}

TEST(Utilities, EnumMap_IteratorAssignment)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        {
            EnumMap<TestEnum1, int>::const_iterator it = s.cbegin();
            assert(it == s.begin());  // Asserts are just to make the value used.

            it = s.cend();
            assert(it == s.cend());

            it = s.end();  // Non-const needs to assignable to const
            assert(it == s.end());

            for (it = s.cbegin(); it != s.cend(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::const_iterator>);
            }

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::const_iterator>);
            }
        }
        {
            EnumMap<TestEnum1, int>::iterator it = s.begin();
            assert(it == s.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // it = s.cend();

            it = s.end();
            assert(it == s.end());

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::iterator>);
            }
        }
        return s;
    }();

    static_assert(s1.size() == 2);
}

TEST(Utilities, EnumMap_Iterator_OffByOneIssues)
{
    constexpr EnumMap<TestEnum1, int> s1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first() == TestEnum1::ONE);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 1)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 2)->second() == 10);
}

TEST(Utilities, EnumMap_Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::THREE, 30});
        s.insert({TestEnum1::FOUR, 40});
        s.insert({TestEnum1::ONE, 10});
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(s1.begin()->first() == TestEnum1::ONE);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == TestEnum1::THREE);
    static_assert(std::next(s1.begin(), 1)->second() == 30);
    static_assert(std::next(s1.begin(), 2)->first() == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 2)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == TestEnum1::THREE);
    static_assert(std::prev(s1.end(), 2)->second() == 30);
    static_assert(std::prev(s1.end(), 3)->first() == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 3)->second() == 10);
}

TEST(Utilities, EnumMap_Find)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.find(TestEnum1::ONE) == s1.cend());
    static_assert(s1.find(TestEnum1::TWO) != s1.cend());
    static_assert(s1.find(TestEnum1::THREE) == s1.cend());
    static_assert(s1.find(TestEnum1::FOUR) != s1.cend());

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(Utilities, EnumMap_MutableFind)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto it = s.find(TestEnum1::TWO);
        it->second() = 25;
        it++;
        it->second() = 45;
        return s;
    }();

    static_assert(s1.at(TestEnum1::TWO) == 25);
    static_assert(s1.at(TestEnum1::FOUR) == 45);
}

TEST(Utilities, EnumMap_Contains)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(Utilities, EnumMap_Count)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.count(TestEnum1::ONE) == 0);
    static_assert(s1.count(TestEnum1::TWO) == 1);
    static_assert(s1.count(TestEnum1::THREE) == 0);
    static_assert(s1.count(TestEnum1::FOUR) == 1);

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(Utilities, EnumMap_RichEnum)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestRichEnum1, int> s{};
        s.insert({TestRichEnum1::C_ONE(), 100});
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(TestRichEnum1::C_ONE()));
    static_assert(!s1.contains(TestRichEnum1::C_TWO()));
}

TEST(Utilities, EnumMap_NonConformingRichEnum)
{
    constexpr auto s1 = []()
    {
        EnumMap<NonConformingTestRichEnum1, int> s{};
        s.insert({NonConformingTestRichEnum1::NC_ONE(), 100});
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(NonConformingTestRichEnum1::NC_ONE()));
    static_assert(!s1.contains(NonConformingTestRichEnum1::NC_TWO()));
}

TEST(Utilities, EnumMap_Equality)
{
    {
        constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::FOUR, 40}, {TestEnum1::ONE, 10}};
        constexpr EnumMap<TestEnum1, int> s3{{TestEnum1::ONE, 10}, {TestEnum1::THREE, 30}};
        constexpr EnumMap<TestEnum1, int> s4{{TestEnum1::ONE, 10}};

        static_assert(s1 == s2);
        static_assert(s2 == s1);

        static_assert(s1 != s3);
        static_assert(s3 != s1);

        static_assert(s1 != s4);
        static_assert(s4 != s1);
    }

    // Values
    {
        constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 44}};
        constexpr EnumMap<TestEnum1, int> s3{{TestEnum1::ONE, 40}, {TestEnum1::FOUR, 10}};

        static_assert(s1 != s2);
        static_assert(s1 != s3);
    }
}

TEST(Utilities, EnumMap_Ranges)
{
    EnumMap<TestRichEnum1, int> s1{{TestRichEnum1::C_ONE(), 10}, {TestRichEnum1::C_FOUR(), 40}};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v.second() == 10; });

    EXPECT_EQ(1, ranges::distance(f));
    int first_entry = f.begin()->second();
    EXPECT_EQ(10, first_entry);
}

TEST(Utilities, EnumMap_NonDefaultConstructible)
{
    constexpr EnumMap<TestEnum1, MockNonDefaultConstructible> s1{};
    static_assert(s1.empty());
}

}  // namespace fixed_containers