#include "ContactStore.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <stdexcept>

namespace
{

JsonValue optionalToJson(const std::optional<std::string>& value)
{
    return value ? JsonValue(*value) : JsonValue();
}

std::optional<std::string> jsonToOptional(const JsonValue& value)
{
    return value.isNull() ? std::nullopt : std::optional<std::string>(value.asString());
}

JsonValue toJson(const Contact& contact)
{
    JsonValue v = JsonValue::makeObject();
    v["firstName"] = JsonValue(contact.firstName);
    v["lastName"] = JsonValue(contact.lastName);
    v["phone"] = JsonValue(contact.phone);
    v["company"] = optionalToJson(contact.company);
    v["dob"] = optionalToJson(contact.dob);
    return v;
}

Contact fromJson(const JsonValue& v)
{
    Contact contact;
    contact.firstName = v.at("firstName").asString();
    contact.lastName = v.at("lastName").asString();
    contact.phone = v.at("phone").asString();
    contact.company = jsonToOptional(v.at("company"));
    contact.dob = jsonToOptional(v.at("dob"));
    return contact;
}

std::string toLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return out;
}

bool isAllDigits(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

} // namespace

bool isValidDob(const std::string& dob)
{
    if (dob.size() != 10 || dob[4] != '-' || dob[7] != '-')
    {
        return false;
    }
    std::string yearStr = dob.substr(0, 4);
    std::string monthStr = dob.substr(5, 2);
    std::string dayStr = dob.substr(8, 2);
    if (!isAllDigits(yearStr) || !isAllDigits(monthStr) || !isAllDigits(dayStr))
    {
        return false;
    }

    int year = std::stoi(yearStr);
    int month = std::stoi(monthStr);
    int day = std::stoi(dayStr);

    std::chrono::year_month_day ymd{
        std::chrono::year{year},
        std::chrono::month{static_cast<unsigned>(month)},
        std::chrono::day{static_cast<unsigned>(day)}
    };
    return ymd.ok();
}

bool isValidPhone(const std::string& phone)
{
    if (phone.size() != 13)
    {
        return false;
    }
    if (phone.substr(0, 3) != "010" || phone[3] != '-' || phone[8] != '-')
    {
        return false;
    }
    return isAllDigits(phone.substr(4, 4)) && isAllDigits(phone.substr(9, 4));
}

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
        std::string fullName = arr[i].at("firstName").asString() + " " + arr[i].at("lastName").asString();
        if (toLower(fullName).find(needle) != std::string::npos)
        {
            result.push_back(i);
        }
    }
    return result;
}
