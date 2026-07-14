#include "ContactStore.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

namespace
{

JsonValue toJson(const Contact& contact)
{
    JsonValue v = JsonValue::makeObject();
    v["name"] = JsonValue(contact.name);
    v["phone"] = JsonValue(contact.phone);
    v["company"] = JsonValue(contact.company);
    v["dob"] = JsonValue(contact.dob);
    return v;
}

Contact fromJson(const JsonValue& v)
{
    Contact contact;
    contact.name = v.at("name").asString();
    contact.phone = v.at("phone").asString();
    contact.company = v.at("company").asString();
    contact.dob = v.at("dob").asString();
    return contact;
}

std::string toLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return out;
}

} // namespace

void ContactStore::load(const std::string& path)
{
    if (std::filesystem::exists(path))
    {
        contacts_ = JsonValue::parseFile(path);
    }
    else
    {
        contacts_ = JsonValue::makeArray();
    }
}

void ContactStore::save(const std::string& path) const
{
    contacts_.saveToFile(path);
}

Contact ContactStore::get(size_t index) const
{
    if (index >= contacts_.size())
    {
        throw std::out_of_range("ContactStore::get: index out of range");
    }
    return fromJson(contacts_.asArray()[index]);
}

size_t ContactStore::add(const Contact& contact)
{
    contacts_.push_back(toJson(contact));
    return contacts_.size() - 1;
}

void ContactStore::update(size_t index, const Contact& contact)
{
    if (index >= contacts_.size())
    {
        throw std::out_of_range("ContactStore::update: index out of range");
    }
    contacts_.asArray()[index] = toJson(contact);
}

void ContactStore::remove(size_t index)
{
    auto& arr = contacts_.asArray();
    if (index >= arr.size())
    {
        throw std::out_of_range("ContactStore::remove: index out of range");
    }
    arr.erase(arr.begin() + static_cast<long>(index));
}

std::vector<size_t> ContactStore::findByName(const std::string& query) const
{
    std::vector<size_t> result;
    std::string needle = toLower(query);
    const auto& arr = contacts_.asArray();
    for (size_t i = 0; i < arr.size(); ++i)
    {
        std::string name = toLower(arr[i].at("name").asString());
        if (name.find(needle) != std::string::npos)
        {
            result.push_back(i);
        }
    }
    return result;
}
