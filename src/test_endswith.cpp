#include <gtest/gtest.h>
#include "fbystring.h"

using namespace std;

TEST(EndsWithWorks,EndsWithWorks)
{
    string indexdothtml("index.html");
    string bladotjs("bla.js");
    string dothtml(".html");
    string dotjs(".js");
    string shortstr("a");

    EXPECT_TRUE(endswith(indexdothtml, dothtml));
    EXPECT_FALSE(endswith(indexdothtml, dotjs));

    EXPECT_FALSE(endswith(bladotjs, dothtml));
    EXPECT_TRUE(endswith(bladotjs, dotjs));

    EXPECT_FALSE(endswith(shortstr,dotjs));
    EXPECT_FALSE(endswith(dothtml, indexdothtml));
    EXPECT_FALSE(endswith(dotjs, indexdothtml));
    EXPECT_FALSE(endswith(dothtml, bladotjs));
    EXPECT_FALSE(endswith(dotjs, bladotjs));
}
