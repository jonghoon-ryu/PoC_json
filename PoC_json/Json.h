#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

enum class JsonType
{
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

class JsonParseError : public std::runtime_error
{
public:
    explicit JsonParseError(const std::string& message) : std::runtime_error(message) {}
};

class JsonValue
{
public:
    JsonValue();
    JsonValue(bool value);
    JsonValue(double value);
    JsonValue(int value);
    JsonValue(const std::string& value);
    JsonValue(const char* value);

    static JsonValue makeArray();
    static JsonValue makeObject();

    JsonType type() const { return type_; }
    bool isNull() const { return type_ == JsonType::Null; }
    bool isBool() const { return type_ == JsonType::Bool; }
    bool isNumber() const { return type_ == JsonType::Number; }
    bool isString() const { return type_ == JsonType::String; }
    bool isArray() const { return type_ == JsonType::Array; }
    bool isObject() const { return type_ == JsonType::Object; }

    bool asBool() const;
    double asNumber() const;
    const std::string& asString() const;

    std::vector<JsonValue>& asArray();
    const std::vector<JsonValue>& asArray() const;
    std::map<std::string, JsonValue>& asObject();
    const std::map<std::string, JsonValue>& asObject() const;

    // Object access. operator[] creates the key (as null) if this is an
    // object and the key is missing; at() throws if this isn't an object
    // or the key is missing.
    JsonValue& operator[](const std::string& key);
    const JsonValue& at(const std::string& key) const;
    bool has(const std::string& key) const;

    // Array access.
    JsonValue& operator[](size_t index);
    const JsonValue& operator[](size_t index) const;
    void push_back(JsonValue value);
    size_t size() const;

    bool operator==(const JsonValue& other) const;
    bool operator!=(const JsonValue& other) const { return !(*this == other); }

    // Pretty-printed (2-space indent) JSON text.
    std::string serialize() const;

    static JsonValue parse(const std::string& text);
    static JsonValue parseFile(const std::string& path);
    void saveToFile(const std::string& path) const;

private:
    void serializeTo(std::string& out, int indent) const;

    JsonType type_ = JsonType::Null;
    bool boolValue_ = false;
    double numberValue_ = 0.0;
    std::string stringValue_;
    std::vector<JsonValue> arrayValue_;
    std::map<std::string, JsonValue> objectValue_;
};
