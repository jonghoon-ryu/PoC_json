// Tests for ContactStore's CRUD operations (see ContactStore.h). The
// interactive console loop in main.cpp isn't unit-tested here directly;
// these tests cover the data-manipulation functions it drives. Only
// compiled/run in Debug builds; main()/RUN_ALL_TESTS() lives in main.cpp.

#ifdef _DEBUG

#include "gmock/gmock.h"

#include <filesystem>

#include "ContactStore.h"

namespace
{

Contact makeContact(const std::string& name)
{
    return Contact{ name, "010-1234-5678", "Acme Corp", "1990-01-01" };
}

TEST(ContactStoreTest, StartsEmpty)
{
    ContactStore store;
    EXPECT_EQ(store.count(), 0u);
}

TEST(ContactStoreTest, AddReturnsIndexAndIncreasesCount)
{
    ContactStore store;
    size_t index = store.add(makeContact("Ada Lovelace"));
    EXPECT_EQ(index, 0u);
    EXPECT_EQ(store.count(), 1u);
    EXPECT_EQ(store.get(0).name, "Ada Lovelace");
}

TEST(ContactStoreTest, GetOutOfRangeThrows)
{
    ContactStore store;
    EXPECT_THROW(store.get(0), std::out_of_range);
}

TEST(ContactStoreTest, UpdateReplacesContactAtIndex)
{
    ContactStore store;
    store.add(makeContact("Ada Lovelace"));
    Contact updated = makeContact("Ada L.");
    updated.company = "Analytical Engines Inc";
    store.update(0, updated);

    Contact result = store.get(0);
    EXPECT_EQ(result.name, "Ada L.");
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
    EXPECT_EQ(store.get(0).name, "Bob");
    EXPECT_EQ(store.get(1).name, "Carol");
}

TEST(ContactStoreTest, RemoveOutOfRangeThrows)
{
    ContactStore store;
    EXPECT_THROW(store.remove(0), std::out_of_range);
}

TEST(ContactStoreTest, FindByNameIsCaseInsensitiveSubstringMatch)
{
    ContactStore store;
    store.add(makeContact("Ada Lovelace"));
    store.add(makeContact("Alan Turing"));
    store.add(makeContact("Grace Hopper"));

    std::vector<size_t> matches = store.findByName("l");
    EXPECT_THAT(matches, testing::ElementsAre(0u, 1u));

    matches = store.findByName("GRACE");
    EXPECT_THAT(matches, testing::ElementsAre(2u));

    matches = store.findByName("nobody");
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
    store.add(makeContact("Ada Lovelace"));
    store.add(makeContact("Alan Turing"));

    const char* path = "contact_store_test_roundtrip.json";
    store.save(path);

    ContactStore reloaded;
    reloaded.load(path);

    ASSERT_EQ(reloaded.count(), 2u);
    EXPECT_EQ(reloaded.get(0).name, "Ada Lovelace");
    EXPECT_EQ(reloaded.get(1).name, "Alan Turing");

    std::filesystem::remove(path);
}

} // namespace

#endif
