#include "ContactTestData.h"

#include <random>

namespace
{

const char* kFirstNames[] = {
    "James", "Mary", "Robert", "Patricia", "John",
    "Jennifer", "Michael", "Linda", "David", "Barbara"
};

const char* kLastNames[] = {
    "Smith", "Johnson", "Williams", "Brown", "Jones",
    "Garcia", "Miller", "Davis", "Rodriguez", "Martinez"
};

const char* kCompanies[] = {
    "Acme Corp", "Globex", "Initech", "Umbrella Inc",
    "Soylent Corp", "Stark Industries", "Wayne Enterprises",
    "Wonka Industries", "Hooli", "Massive Dynamic"
};

std::string makePhoneNumber(std::mt19937& rng)
{
    std::uniform_int_distribution<int> digit(0, 9);
    std::string phone = "010-";
    for (int i = 0; i < 4; ++i) phone += static_cast<char>('0' + digit(rng));
    phone += '-';
    for (int i = 0; i < 4; ++i) phone += static_cast<char>('0' + digit(rng));
    return phone;
}

std::string makeDob(std::mt19937& rng)
{
    std::uniform_int_distribution<int> yearDist(1960, 2005);
    std::uniform_int_distribution<int> monthDist(1, 12);
    std::uniform_int_distribution<int> dayDist(1, 28);
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", yearDist(rng), monthDist(rng), dayDist(rng));
    return buf;
}

} // namespace

JsonValue makeRandomContacts(int count)
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> firstNameDist(0, static_cast<int>(std::size(kFirstNames)) - 1);
    std::uniform_int_distribution<int> lastNameDist(0, static_cast<int>(std::size(kLastNames)) - 1);
    std::uniform_int_distribution<int> companyDist(0, static_cast<int>(std::size(kCompanies)) - 1);

    JsonValue contacts = JsonValue::makeArray();
    for (int i = 0; i < count; ++i)
    {
        JsonValue contact = JsonValue::makeObject();
        std::string name = std::string(kFirstNames[firstNameDist(rng)]) + " " + kLastNames[lastNameDist(rng)];
        contact["name"] = JsonValue(name);
        contact["phone"] = JsonValue(makePhoneNumber(rng));
        contact["company"] = JsonValue(kCompanies[companyDist(rng)]);
        contact["dob"] = JsonValue(makeDob(rng));
        contacts.push_back(contact);
    }
    return contacts;
}
