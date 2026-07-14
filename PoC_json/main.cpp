#include <iostream>

#include "ContactTestData.h"
#include "Json.h"

#ifdef _DEBUG
#include <gmock/gmock.h>

using namespace testing;

int main(void)
{
    InitGoogleMock();
    return RUN_ALL_TESTS();
}
#else

int main(void)
{
    JsonValue contacts = makeRandomContacts(10);
    contacts.saveToFile("contacts.json");
    std::cout << "Saved " << contacts.size() << " contacts to contacts.json\n";

    JsonValue loaded = JsonValue::parseFile("contacts.json");
    std::cout << "Loaded back " << loaded.size() << " contacts:\n";
    for (size_t i = 0; i < loaded.size(); ++i)
    {
        const JsonValue& contact = loaded[i];
        std::cout << (i + 1) << ". " << contact.at("name").asString()
            << " | " << contact.at("phone").asString()
            << " | " << contact.at("company").asString()
            << " | " << contact.at("dob").asString() << "\n";
    }
    return 0;
}
#endif