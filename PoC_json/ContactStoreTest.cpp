// Tests for ContactStore's CRUD operations and DOB validation (see
// ContactStore.h). The interactive console loop in main.cpp isn't
// unit-tested here directly; these tests cover the data-manipulation
// functions it drives. Only compiled/run in Debug builds; main()/
// RUN_ALL_TESTS() lives in main.cpp.

#ifdef _DEBUG

#include "gmock/gmock.h"

#include <filesystem>

#include "ContactStore.h"

namespace
{

Contact makeContact(const std::string& firstName, const std::string& lastName = "Doe")
{
    return Contact{ firstName, lastName, "010-1234-5678", "Acme Corp", "1990-01-01" };
}

TEST(IsValidDobTest, AcceptsRealDates)
{
    EXPECT_TRUE(isValidDob("1990-01-01"));
    EXPECT_TRUE(isValidDob("2000-02-29")); // leap year
    EXPECT_TRUE(isValidDob("1999-12-31"));
}

TEST(IsValidDobTest, RejectsInvalidCalendarDates)
{
    EXPECT_FALSE(isValidDob("1900-00-00"));
    EXPECT_FALSE(isValidDob("2021-02-30")); // February has 28/29 days
    EXPECT_FALSE(isValidDob("2021-13-01")); // no month 13
    EXPECT_FALSE(isValidDob("2021-04-31")); // April has 30 days
    EXPECT_FALSE(isValidDob("2021-02-29")); // not a leap year
}

TEST(IsValidDobTest, RejectsMalformedStrings)
{
    EXPECT_FALSE(isValidDob(""));
    EXPECT_FALSE(isValidDob("1990/01/01"));
    EXPECT_FALSE(isValidDob("90-01-01"));
    EXPECT_FALSE(isValidDob("not a date"));
}

TEST(IsValidPhoneTest, AcceptsKoreanMobileFormat)
{
    EXPECT_TRUE(isValidPhone("010-1234-5678"));
    EXPECT_TRUE(isValidPhone("010-0000-0000"));
}

TEST(IsValidPhoneTest, RejectsWrongPrefixOrGrouping)
{
    EXPECT_FALSE(isValidPhone("011-1234-5678")); // not 010
    EXPECT_FALSE(isValidPhone("010-123-5678")); // only 3 digits in middle group
    EXPECT_FALSE(isValidPhone("010-1234-567")); // only 3 digits in last group
    EXPECT_FALSE(isValidPhone("01012345678")); // missing hyphens
    EXPECT_FALSE(isValidPhone("010.1234.5678")); // wrong separators
}

TEST(IsValidPhoneTest, RejectsMalformedStrings)
{
    EXPECT_FALSE(isValidPhone(""));
    EXPECT_FALSE(isValidPhone("not a phone"));
    EXPECT_FALSE(isValidPhone("010-abcd-5678"));
}

TEST(ContactStoreTest, StartsEmpty)
{
    ContactStore store;
    EXPECT_EQ(store.count(), 0u);
}

TEST(ContactStoreTest, AddReturnsIndexAndIncreasesCount)
{
    ContactStore store;
    size_t index = store.add(makeContact("Ada", "Lovelace"));
    EXPECT_EQ(index, 0u);
    EXPECT_EQ(store.count(), 1u);
    EXPECT_EQ(store.get(0).firstName, "Ada");
    EXPECT_EQ(store.get(0).lastName, "Lovelace");
}

TEST(ContactStoreTest, GetOutOfRangeThrows)
{
    ContactStore store;
    EXPECT_THROW(store.get(0), std::out_of_range);
}

TEST(ContactStoreTest, UpdateReplacesContactAtIndex)
{
    ContactStore store;
    store.add(makeContact("Ada", "Lovelace"));
    Contact updated = makeContact("Ada", "L.");
    updated.company = "Analytical Engines Inc";
    store.update(0, updated);

    Contact result = store.get(0);
    EXPECT_EQ(result.lastName, "L.");
    EXPECT_EQ(result.company, "Analytical Engines Inc");
}

TEST(ContactStoreTest, UpdateOutOfRangeThrows)
{
    ContactStore store;
    EXPECT_THROW(store.update(0, makeContact("Nobody")), std::out_of_range);
}

TEST(ContactStoreTest, RemoveShiftsLaterIndicesDown)
{
    ContactStore store;
    store.add(makeContact("Alice"));
    store.add(makeContact("Bob"));
    store.add(makeContact("Carol"));

    store.remove(0);

    ASSERT_EQ(store.count(), 2u);
    EXPECT_EQ(store.get(0).firstName, "Bob");
    EXPECT_EQ(store.get(1).firstName, "Carol");
}

TEST(ContactStoreTest, RemoveOutOfRangeThrows)
{
    ContactStore store;
    EXPECT_THROW(store.remove(0), std::out_of_range);
}

TEST(ContactStoreTest, FindByNameIsCaseInsensitiveSubstringMatch)
{
    ContactStore store;
    store.add(makeContact("Ada", "Lovelace"));
    store.add(makeContact("Alan", "Turing"));
    store.add(makeContact("Grace", "Hopper"));

    std::vector<size_t> matches = store.findByName("l");
    EXPECT_THAT(matches, testing::ElementsAre(0u, 1u));

    matches = store.findByName("GRACE");
    EXPECT_THAT(matches, testing::ElementsAre(2u));

    matches = store.findByName("nobody");
    EXPECT_TRUE(matches.empty());
}

TEST(ContactStoreTest, FindByPhoneSubstringMatch)
{
    ContactStore store;
    Contact ada = makeContact("Ada", "Lovelace");
    ada.phone = "010-1111-2222";
    Contact alan = makeContact("Alan", "Turing");
    alan.phone = "010-1111-3333";
    Contact grace = makeContact("Grace", "Hopper");
    grace.phone = "010-9999-9999";
    store.add(ada);
    store.add(alan);
    store.add(grace);

    std::vector<size_t> matches = store.findByPhone("1111");
    EXPECT_THAT(matches, testing::ElementsAre(0u, 1u));

    matches = store.findByPhone("010-9999-9999");
    EXPECT_THAT(matches, testing::ElementsAre(2u));

    matches = store.findByPhone("010-0000-0000");
    EXPECT_TRUE(matches.empty());
}

TEST(ContactStoreTest, LoadFromMissingFileStartsEmpty)
{
    ContactStore store;
    store.load("contact_store_test_missing.json");
    EXPECT_EQ(store.count(), 0u);
}

TEST(ContactStoreTest, SaveThenLoadRoundTrips)
{
    ContactStore store;
    store.add(makeContact("Ada", "Lovelace"));
    store.add(makeContact("Alan", "Turing"));

    const char* path = "contact_store_test_roundtrip.json";
    store.save(path);

    ContactStore reloaded;
    reloaded.load(path);

    ASSERT_EQ(reloaded.count(), 2u);
    EXPECT_EQ(reloaded.get(0).firstName, "Ada");
    EXPECT_EQ(reloaded.get(1).firstName, "Alan");

    std::filesystem::remove(path);
}

TEST(ContactStoreTest, OptionalFieldsRoundTripAsNullWhenSkipped)
{
    ContactStore store;
    Contact c = makeContact("Ada", "Lovelace");
    c.company = std::nullopt;
    c.dob = std::nullopt;
    store.add(c);

    const char* path = "contact_store_test_optional_roundtrip.json";
    store.save(path);

    ContactStore reloaded;
    reloaded.load(path);

    ASSERT_EQ(reloaded.count(), 1u);
    EXPECT_EQ(reloaded.get(0).company, std::nullopt);
    EXPECT_EQ(reloaded.get(0).dob, std::nullopt);

    std::filesystem::remove(path);
}

} // namespace

#endif
