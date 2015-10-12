/**
 * @file
 * @author Hamik Mukelyan
 *
 * Tests that the JSON parser library can be used. A lot of the code here has
 * been pulled from the rapidjson example code.
 */

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <string.h>

using namespace std;
using namespace rapidjson;

#ifndef TEST_JSONLIB_CPP
#define TEST_JSONLIB_CPP

/*
 * This is a "test fixture" that sets up things we need in the actual unit
 * tests below. Note that an object of this class is created before
 * each test case begins and is torn down when each test case ends.
 */
class jsonlibTest : public ::testing::Test {
protected:

	string jsonstr;

	virtual void SetUp() {
		jsonstr = " { \"hello\" : \"world\", \"t\" : true , "
				"\"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, "
				"\"a\":[1, 2, 3, 4] } ";
	}

	virtual void TearDown() { }
};

/*
 * This test mirrors rapidjson example code. Just included it here to make
 * sure the library behaves as expected.
 */
TEST_F(jsonlibTest, ParseToDocument) {
    Document document;
    char buffer[jsonstr.length() + 1];
    memcpy(buffer, jsonstr.c_str(), jsonstr.length());
    buffer[jsonstr.length()] = 0; // string has to be null-terminated
    ASSERT_FALSE(document.ParseInsitu(buffer).HasParseError());
}

/*
 * This test mirrors rapidjson example code. Just included it here to make
 * sure the library behaves as expected.
 */
TEST_F(jsonlibTest, AccessValuesInDocument) {
    Document document;
    char buffer[jsonstr.length() + 1];
    memcpy(buffer, jsonstr.c_str(), jsonstr.length());
    buffer[jsonstr.length()] = 0; // string has to be null-terminated
    ASSERT_FALSE(document.ParseInsitu(buffer).HasParseError());

    // document is root of dom. can be object or array
    ASSERT_TRUE(document.IsObject());

    ASSERT_TRUE(document.HasMember("hello"));
    ASSERT_TRUE(document["hello"].IsString());
    ASSERT_STREQ("world", document["hello"].GetString());

    // Since v0.2 can use single lookup to check existence and value
    Value::MemberIterator hello = document.FindMember("hello");
    ASSERT_TRUE(hello != document.MemberEnd());
    ASSERT_TRUE(hello->value.IsString());
    ASSERT_STREQ("world", hello->value.GetString());;

    ASSERT_TRUE(document["t"].IsBool());
    ASSERT_TRUE(document["t"].GetBool());

    ASSERT_TRUE(document["f"].IsBool());
    ASSERT_FALSE(document["f"].GetBool());

    ASSERT_TRUE(document["n"].IsNull());

    ASSERT_TRUE(document["i"].IsNumber());
    ASSERT_TRUE(document["i"].IsInt());
    ASSERT_EQ(123, document["i"].GetInt());

    ASSERT_TRUE(document["pi"].IsNumber());
    ASSERT_TRUE(document["pi"].IsDouble());
    ASSERT_EQ(3.1416, document["pi"].GetDouble());

    {
        const Value& a = document["a"];
        ASSERT_TRUE(a.IsArray());
        // rapidjson uses SizeType instead of size_t.
        for (SizeType i = 0; i < a.Size(); i++)
            ASSERT_EQ(i + 1, a[i].GetInt());

        // Iterating array with iterators
        int i = 1;
        for(Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr) {
        	ASSERT_EQ(i, itr->GetInt());
        	i++;
        }
    }
}

#endif // TEST_JSONLIB_CPP
