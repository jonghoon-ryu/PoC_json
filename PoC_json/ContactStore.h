#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Json.h"

struct Contact
{
    std::string firstName;
    std::string lastName;
    std::string phone;
    std::optional<std::string> company;
    std::optional<std::string> dob;
};

// Returns true if dob is a real calendar date in "YYYY-MM-DD" format
// (rejects malformed strings and non-existent dates like "1900-00-00" or
// "2021-02-30").
bool isValidDob(const std::string& dob);

// Returns true if phone matches the Korean mobile format "010-XXXX-XXXX"
// (exactly that prefix and digit grouping).
bool isValidPhone(const std::string& phone);

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

    // Case-insensitive substring match on "firstName lastName"; returns
    // matching indices.
    std::vector<size_t> findByName(const std::string& query) const;

    // Substring match on phone; returns matching indices.
    std::vector<size_t> findByPhone(const std::string& query) const;

private:
    JsonValue contacts_ = JsonValue::makeArray();
};
