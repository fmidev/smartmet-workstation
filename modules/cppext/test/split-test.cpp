#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cppext/split.h"

#include <type_traits>

#include <vector>
#include <list>

#include <fstream>

using namespace cppext;
using namespace std;
using namespace testing;

template<typename T>
struct is_vector : false_type
{
};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : true_type
{
};

TEST(Split, parseLongishString)
{
    auto longStringF = ifstream{"long-file-to-split"};
    auto buffer = stringstream{};
    buffer << longStringF.rdbuf();
    auto longString = buffer.str();
    auto result = split(longString, ":");
}

TEST(Split, DefaultReturnTypeIsVector)
{
	auto returnType = split(string{"a,b,c"}, string{ "," });

	EXPECT_TRUE(is_vector<decltype(returnType)>());
}

//TEST(Split, DoesNotAllowContainersWithDifferentValueType)
//{
//	split<string, vector<const char*>>(string{ "a,b,c" }, string{ "," });
//}

TEST(Split, SplittingRValueStringWithRValueCharIsSupported)
{
	auto splitted = split(string{ "a,b,c,d,e" }, ',');

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueStringWithRValueCharIsSupported)
{
	auto toSplit = string{ "a,b,c,d,e" };

	auto splitted = split(toSplit, ',');

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingRValueStringWithLValueCharIsSupported)
{
	char delim = ',';

	auto splitted = split(string{ "a,b,c,d,e" }, delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueStringWithLValueCharIsSupported)
{
	auto toSplit = string{ "a,b,c,d,e" };
	char delim = ',';

	auto splitted = split(toSplit, delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingRValueStringWithRValueStringIsSupported)
{
	auto splitted = split(string{ "a,b,c,d,e" }, string{ "," });

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingRValueStringWithLValueStringIsSupported)
{
	auto delim = string{","};

	auto splitted = split(string{ "a,b,c,d,e" }, delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueStringWithRValueStringIsSupported)
{
	auto toSplit = string{ "a,b,c,d,e" };

	auto splitted = split(toSplit, string{ "," });

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueStringWithLValueStringIsSupported)
{
	auto toSplit = string{ "a,b,c,d,e" };
	auto delim = string{ "," };

	auto splitted = split(toSplit, delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingConstCharWithConstCharIsSupported)
{
	auto splitted = split("a,b,c,d,e", ",");

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueConstCharWithLValueConstCharIsSupported)
{
    auto toSplit = "a,b,c,d,e";
    auto delim = ",";
    auto splitted = split(toSplit, delim);

    EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingRValueStringWithConstCharIsSupported)
{
	auto splitted = split(string{ "a,b,c,d,e" }, ",");

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingLValueStringWithConstCharIsSupported)
{
	auto toSplit = string{ "a,b,c,d,e" };
	auto splitted = split(toSplit, ",");

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingConstCharWithRValueStringIsSupported)
{
	auto splitted = split("a,b,c,d,e", string{ "," });

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingConstCharWithLValueStringIsSupported)
{
	auto delim = string{ "," };
	auto splitted = split("a,b,c,d,e", delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

TEST(Split, SplittingConstCharWithLValueCharIsSupported)
{
    auto delim = ',';
	auto splitted = split("a,b,c,d,e", delim);

	EXPECT_THAT(splitted.size(), Eq(5));
}

////////////////////////////////////////////////////////////////////////////
//// Passing container is very awkward as is shown below
//
//TEST(Split, ListCanBeUsedAsReturnedContainerType1)
//{
//	auto splitted = split<list<string>>("a, b, c, d, e", ",");
//
//	EXPECT_THAT(splitted.size(), Eq(5));
//}
//
//TEST(Split, ListCanBeUsedAsReturnedContainerType2)
//{
//	auto splitted = split<string, string, list<string>>(string{ "a, b, c, d, e" }, string{ "," });
//
//	EXPECT_THAT(splitted.size(), Eq(5));
//}
//
//TEST(Split, ListCanBeUsedAsReturnedContainerType3)
//{
//	auto splitted = split<string, string, list<string>>("a, b, c, d, e", string{ "," });
//
//	EXPECT_THAT(splitted.size(), Eq(5));
//}
//
//TEST(Split, ListCanBeUsedAsReturnedContainerType4)
//{
//	auto splitted = split<string, list<string>>("a, b, c, d, e", string{ "," });
//	// auto splitted = split<const char*,string, list<string>>("a, b, c, d, e", string{ "," }); This doesnt work. User will never learn these...
//
//	EXPECT_THAT(splitted.size(), Eq(5));
//}
