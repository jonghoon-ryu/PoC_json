// Tests for the hand-written JSON parser/writer (Json.h) and the
// random-contact test-data generator (ContactTestData.h). Only compiled/run
// in Debug builds; main()/RUN_ALL_TESTS() lives in main.cpp.

#ifdef _DEBUG

#include "gmock/gmock.h"

#include <filesystem>

#include "ContactTestData.h"
#include "Json.h"

namespace
{

TEST(JsonValueTest, ParsesNull)
{
    JsonValue v = JsonValue::parse("null");
    EXPECT_TRUE(v.isNull());
}

TEST(JsonValueTest, ParsesBooleans)
{
    EXPECT_TRUE(JsonValue::parse("true").asBool());
    EXPECT_FALSE(JsonValue::parse("false").asBool());
}

TEST(JsonValueTest, ParsesNumbers)
{
    EXPECT_EQ(JsonValue::parse("42").asNumber(), 42.0);
    EXPECT_EQ(JsonValue::parse("-3.5").asNumber(), -3.5);
    EXPECT_EQ(JsonValue::parse("1.5e2").asNumber(), 150.0);
}

TEST(JsonValueTest, ParsesStringsWithEscapes)
{
    JsonValue v = JsonValue::parse("\"line1\\nline2\\t\\\"quoted\\\"\"");
    EXPECT_EQ(v.asString(), "line1\nline2\t\"quoted\"");
}

TEST(JsonValueTest, ParsesEmptyArrayAndObject)
{
    EXPECT_EQ(JsonValue::parse("[]").size(), 0u);
    EXPECT_EQ(JsonValue::parse("{}").size(), 0u);
}

TEST(JsonValueTest, ParsesNestedArraysAndObjects)
{
    JsonValue v = JsonValue::parse(R"({"a":[1,2,{"b":true,"c":null}]})");
    ASSERT_TRUE(v.isObject());
    const JsonValue& arr = v.at("a");
    ASSERT_TRUE(arr.isArray());
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0].asNumber(), 1.0);
    EXPECT_EQ(arr[1].asNumber(), 2.0);
    const JsonValue& nested = arr[2];
    EXPECT_TRUE(nested.at("b").asBool());
    EXPECT_TRUE(nested.at("c").isNull());
}

TEST(JsonValueTest, RejectsMalformedJson)
{
    EXPECT_THROW(JsonValue::parse("{"), JsonParseError);
    EXPECT_THROW(JsonValue::parse("[1,]"), JsonParseError);
    EXPECT_THROW(JsonValue::parse("not json"), JsonParseError);
}

TEST(JsonValueTest, SerializeThenParseRoundTrips)
{
    JsonValue original = JsonValue::makeObject();
    original["name"] = JsonValue("Ada Lovelace");
    original["age"] = JsonValue(36);
    original["active"] = JsonValue(true);
    original["notes"] = JsonValue();
    JsonValue tags = JsonValue::makeArray();
    tags.push_back(JsonValue("math"));
    tags.push_back(JsonValue("engineering"));
    original["tags"] = tags;

    std::string text = original.serialize();
    JsonValue reparsed = JsonValue::parse(text);

    EXPECT_EQ(original, reparsed);
}

TEST(ContactTestDataTest, GeneratesRequestedCount)
{
    JsonValue contacts = makeRandomContacts(10);
    ASSERT_TRUE(contacts.isArray());
    EXPECT_EQ(contacts.size(), 10u);
    for (size_t i = 0; i < contacts.size(); ++i)
    {
        const JsonValue& contact = contacts[i];
        EXPECT_TRUE(contact.has("name"));
        EXPECT_TRUE(contact.has("phone"));
        EXPECT_TRUE(contact.has("company"));
        EXPECT_TRUE(contact.has("dob"));
    }
}

TEST(ContactTestDataTest, SaveAndLoadFileRoundTrips)
{
    JsonValue contacts = makeRandomContacts(10);
    const char* path = "contacts_test_roundtrip.json";
    contacts.saveToFile(path);

    JsonValue loaded = JsonValue::parseFile(path);
    EXPECT_EQ(contacts, loaded);

    std::filesystem::remove(path);
}

} // namespace

#endif
