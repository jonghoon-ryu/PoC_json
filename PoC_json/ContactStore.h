#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Json.h"

struct Contact
{
    std::string name;
    std::string phone;
    std::string company;
    std::string dob;
};

// Manages an in-memory contact list backed by a JSON file (via JsonLib).
// Indices are positions in the current list, valid until the next add/
// remove call.
class ContactStore
{
public:
    // Loads the list from path, or starts empty if the file doesn't exist.
    void load(const std::string& path);
    void save(const std::string& path) const;

    size_t count() const { return contacts_.size(); }
    Contact get(size_t index) const;

    // Returns the new contact's index.
    size_t add(const Contact& contact);
    void update(size_t index, const Contact& contact);
    void remove(size_t index);

    // Case-insensitive substring match on name; returns matching indices.
    std::vector<size_t> findByName(const std::string& query) const;

private:
    JsonValue contacts_ = JsonValue::makeArray();
};
