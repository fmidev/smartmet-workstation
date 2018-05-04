#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmsSupport/AssertionMacros.h"
#include "WmsSupport/BaseQuery.h"

using namespace testing;
using namespace std;
using namespace Wms;

MXT_GENERATE_HAS_MEMBER(c_str)

TEST(HasMember, HasMemberReturnsTrueWhenMemberIsFound)
{
    EXPECT_TRUE(has_member_c_str<string>());
}

TEST(HasMember, HasMemberReturnsFalseWhenMemberIsNotFound)
{
    EXPECT_FALSE(has_member_c_str<BaseQuery>());
}
