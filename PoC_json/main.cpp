#include <iostream>

#ifdef _DEBUG
#include <gmock/gmock.h>

using namespace testing;

int main(void)
{
    InitGoogleMock();
    return RUN_ALL_TESTS();
}
#else

#include <string>

#include "ContactStore.h"

namespace
{

const char* kDataFile = "contacts.json";

std::string prompt(const std::string& label)
{
    std::cout << label;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void printContact(size_t index, const Contact& c)
{
    std::cout << index << ". " << c.firstName << " " << c.lastName << " | " << c.phone
        << " | " << c.company.value_or("NA") << " | " << c.dob.value_or("NA") << "\n";
}

void listAll(const ContactStore& store)
{
    if (store.count() == 0)
    {
        std::cout << "(no contacts)\n";
        return;
    }
    for (size_t i = 0; i < store.count(); ++i)
    {
        printContact(i, store.get(i));
    }
}

// Reads a 1/2 choice ("1. Enter value" / "2. Skip"), re-prompting until one
// of those is given. Returns true for "1" (enter), false for "2" (skip).
bool promptEnterOrSkip()
{
    while (true)
    {
        std::cout << "  1. Enter value\n"
            << "  2. Skip\n";
        std::string choice = prompt("Choice (1/2): ");
        if (choice == "1")
        {
            return true;
        }
        if (choice == "2")
        {
            return false;
        }
        std::cout << "Please enter 1 or 2.\n";
    }
}

std::optional<std::string> promptCompany()
{
    std::cout << "Company:\n";
    if (!promptEnterOrSkip())
    {
        return std::nullopt;
    }
    return prompt("Company: ");
}

std::optional<std::string> promptDob()
{
    std::cout << "DOB:\n";
    if (!promptEnterOrSkip())
    {
        return std::nullopt;
    }
    while (true)
    {
        std::string dob = prompt("DOB (YYYY-MM-DD): ");
        if (isValidDob(dob))
        {
            return dob;
        }
        std::cout << "Invalid date, please try again.\n";
    }
}

std::string promptPhone()
{
    while (true)
    {
        std::string phone = prompt("Phone (010-XXXX-XXXX): ");
        if (isValidPhone(phone))
        {
            return phone;
        }
        std::cout << "Invalid phone number, please try again.\n";
    }
}

Contact readContactFromInput()
{
    Contact c;
    c.firstName = prompt("First name: ");
    c.lastName = prompt("Last name: ");
    c.phone = promptPhone();
    c.company = promptCompany();
    c.dob = promptDob();
    return c;
}

// Prints the list and reads a valid index from the user; returns false
// (with a message already printed) if there's nothing to select or the
// input isn't a valid index.
bool readIndex(const ContactStore& store, size_t& outIndex)
{
    if (store.count() == 0)
    {
        std::cout << "(no contacts)\n";
        return false;
    }
    listAll(store);
    std::string line = prompt("Index: ");
    try
    {
        int idx = std::stoi(line);
        if (idx < 0 || static_cast<size_t>(idx) >= store.count())
        {
            std::cout << "Out of range.\n";
            return false;
        }
        outIndex = static_cast<size_t>(idx);
        return true;
    }
    catch (const std::exception&)
    {
        std::cout << "Invalid index.\n";
        return false;
    }
}

void addContact(ContactStore& store)
{
    Contact c = readContactFromInput();
    size_t index = store.add(c);
    std::cout << "Added at index " << index << ".\n";
}

void updateContact(ContactStore& store)
{
    size_t index;
    if (!readIndex(store, index))
    {
        return;
    }
    std::cout << "Enter new values:\n";
    Contact c = readContactFromInput();
    store.update(index, c);
    std::cout << "Updated index " << index << ".\n";
}

void deleteContact(ContactStore& store)
{
    size_t index;
    if (!readIndex(store, index))
    {
        return;
    }
    store.remove(index);
    std::cout << "Deleted index " << index << ".\n";
}

void findByName(const ContactStore& store)
{
    std::string query = prompt("Name contains: ");
    std::vector<size_t> matches = store.findByName(query);
    if (matches.empty())
    {
        std::cout << "No matches.\n";
        return;
    }
    for (size_t index : matches)
    {
        printContact(index, store.get(index));
    }
}

void printMenu(const ContactStore& store)
{
    std::cout << "\n=== Contact List (" << store.count() << ") ===\n"
        << "1. List all\n"
        << "2. Add contact\n"
        << "3. Update contact\n"
        << "4. Delete contact\n"
        << "5. Find by name\n"
        << "0. Exit\n"
        << "> ";
}

} // namespace

int main(void)
{
    ContactStore store;
    store.load(kDataFile);

    while (true)
    {
        printMenu(store);
        std::string line;
        if (!std::getline(std::cin, line))
        {
            break;
        }

        int choice;
        try
        {
            choice = std::stoi(line);
        }
        catch (const std::exception&)
        {
            std::cout << "Please enter a number.\n";
            continue;
        }

        if (choice == 0)
        {
            break;
        }
        else if (choice == 1)
        {
            listAll(store);
        }
        else if (choice == 2)
        {
            addContact(store);
            store.save(kDataFile);
        }
        else if (choice == 3)
        {
            updateContact(store);
            store.save(kDataFile);
        }
        else if (choice == 4)
        {
            deleteContact(store);
            store.save(kDataFile);
        }
        else if (choice == 5)
        {
            findByName(store);
        }
        else
        {
            std::cout << "Unknown option.\n";
        }
    }

    store.save(kDataFile);
    std::cout << "Goodbye.\n";
    return 0;
}
#endif
