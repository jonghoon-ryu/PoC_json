#include "Json.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

JsonValue::JsonValue() : type_(JsonType::Null) {}

JsonValue::JsonValue(bool value) : type_(JsonType::Bool), boolValue_(value) {}

JsonValue::JsonValue(double value) : type_(JsonType::Number), numberValue_(value) {}

JsonValue::JsonValue(int value) : type_(JsonType::Number), numberValue_(static_cast<double>(value)) {}

JsonValue::JsonValue(const std::string& value) : type_(JsonType::String), stringValue_(value) {}

JsonValue::JsonValue(const char* value) : type_(JsonType::String), stringValue_(value) {}

JsonValue JsonValue::makeArray()
{
    JsonValue v;
    v.type_ = JsonType::Array;
    return v;
}

JsonValue JsonValue::makeObject()
{
    JsonValue v;
    v.type_ = JsonType::Object;
    return v;
}

bool JsonValue::asBool() const
{
    if (type_ != JsonType::Bool)
    {
        throw JsonParseError("JsonValue is not a bool");
    }
    return boolValue_;
}

double JsonValue::asNumber() const
{
    if (type_ != JsonType::Number)
    {
        throw JsonParseError("JsonValue is not a number");
    }
    return numberValue_;
}

const std::string& JsonValue::asString() const
{
    if (type_ != JsonType::String)
    {
        throw JsonParseError("JsonValue is not a string");
    }
    return stringValue_;
}

std::vector<JsonValue>& JsonValue::asArray()
{
    if (type_ != JsonType::Array)
    {
        throw JsonParseError("JsonValue is not an array");
    }
    return arrayValue_;
}

const std::vector<JsonValue>& JsonValue::asArray() const
{
    if (type_ != JsonType::Array)
    {
        throw JsonParseError("JsonValue is not an array");
    }
    return arrayValue_;
}

std::map<std::string, JsonValue>& JsonValue::asObject()
{
    if (type_ != JsonType::Object)
    {
        throw JsonParseError("JsonValue is not an object");
    }
    return objectValue_;
}

const std::map<std::string, JsonValue>& JsonValue::asObject() const
{
    if (type_ != JsonType::Object)
    {
        throw JsonParseError("JsonValue is not an object");
    }
    return objectValue_;
}

JsonValue& JsonValue::operator[](const std::string& key)
{
    if (type_ == JsonType::Null)
    {
        type_ = JsonType::Object;
    }
    if (type_ != JsonType::Object)
    {
        throw JsonParseError("JsonValue is not an object");
    }
    return objectValue_[key];
}

const JsonValue& JsonValue::at(const std::string& key) const
{
    const auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end())
    {
        throw JsonParseError("JsonValue object has no key: " + key);
    }
    return it->second;
}

bool JsonValue::has(const std::string& key) const
{
    return type_ == JsonType::Object && objectValue_.find(key) != objectValue_.end();
}

JsonValue& JsonValue::operator[](size_t index)
{
    return asArray().at(index);
}

const JsonValue& JsonValue::operator[](size_t index) const
{
    return asArray().at(index);
}

void JsonValue::push_back(JsonValue value)
{
    asArray().push_back(std::move(value));
}

size_t JsonValue::size() const
{
    if (type_ == JsonType::Array)
    {
        return arrayValue_.size();
    }
    if (type_ == JsonType::Object)
    {
        return objectValue_.size();
    }
    throw JsonParseError("JsonValue has no size()");
}

bool JsonValue::operator==(const JsonValue& other) const
{
    if (type_ != other.type_)
    {
        return false;
    }
    switch (type_)
    {
    case JsonType::Null:
        return true;
    case JsonType::Bool:
        return boolValue_ == other.boolValue_;
    case JsonType::Number:
        return numberValue_ == other.numberValue_;
    case JsonType::String:
        return stringValue_ == other.stringValue_;
    case JsonType::Array:
        return arrayValue_ == other.arrayValue_;
    case JsonType::Object:
        return objectValue_ == other.objectValue_;
    }
    return false;
}

namespace
{

void escapeStringTo(std::string& out, const std::string& s)
{
    out += '"';
    for (char c : s)
    {
        switch (c)
        {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            }
            else
            {
                out += c;
            }
        }
    }
    out += '"';
}

void appendNumber(std::string& out, double value)
{
    if (std::isfinite(value) && value == static_cast<long long>(value) &&
        std::abs(value) < 1e15)
    {
        out += std::to_string(static_cast<long long>(value));
    }
    else
    {
        std::ostringstream oss;
        oss << value;
        out += oss.str();
    }
}

void appendIndent(std::string& out, int indent)
{
    out.append(static_cast<size_t>(indent) * 2, ' ');
}

} // namespace

void JsonValue::serializeTo(std::string& out, int indent) const
{
    switch (type_)
    {
    case JsonType::Null:
        out += "null";
        break;
    case JsonType::Bool:
        out += boolValue_ ? "true" : "false";
        break;
    case JsonType::Number:
        appendNumber(out, numberValue_);
        break;
    case JsonType::String:
        escapeStringTo(out, stringValue_);
        break;
    case JsonType::Array:
        if (arrayValue_.empty())
        {
            out += "[]";
            break;
        }
        out += "[\n";
        for (size_t i = 0; i < arrayValue_.size(); ++i)
        {
            appendIndent(out, indent + 1);
            arrayValue_[i].serializeTo(out, indent + 1);
            if (i + 1 < arrayValue_.size())
            {
                out += ',';
            }
            out += '\n';
        }
        appendIndent(out, indent);
        out += ']';
        break;
    case JsonType::Object:
        if (objectValue_.empty())
        {
            out += "{}";
            break;
        }
        out += "{\n";
        {
            size_t i = 0;
            for (const auto& [key, value] : objectValue_)
            {
                appendIndent(out, indent + 1);
                escapeStringTo(out, key);
                out += ": ";
                value.serializeTo(out, indent + 1);
                if (++i < objectValue_.size())
                {
                    out += ',';
                }
                out += '\n';
            }
        }
        appendIndent(out, indent);
        out += '}';
        break;
    }
}

std::string JsonValue::serialize() const
{
    std::string out;
    serializeTo(out, 0);
    return out;
}

namespace
{

class JsonParser
{
public:
    explicit JsonParser(const std::string& text) : text_(text) {}

    JsonValue parse()
    {
        JsonValue v = parseValue();
        skipWhitespace();
        if (pos_ != text_.size())
        {
            fail("unexpected trailing characters");
        }
        return v;
    }

private:
    const std::string& text_;
    size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& message) const
    {
        throw JsonParseError("JSON parse error at offset " + std::to_string(pos_) + ": " + message);
    }

    char peek() const
    {
        if (pos_ >= text_.size())
        {
            fail("unexpected end of input");
        }
        return text_[pos_];
    }

    char next()
    {
        char c = peek();
        ++pos_;
        return c;
    }

    void expect(char c)
    {
        if (next() != c)
        {
            fail(std::string("expected '") + c + "'");
        }
    }

    void skipWhitespace()
    {
        while (pos_ < text_.size())
        {
            char c = text_[pos_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            {
                ++pos_;
            }
            else
            {
                break;
            }
        }
    }

    bool consumeLiteral(const char* literal)
    {
        size_t len = strlen(literal);
        if (text_.compare(pos_, len, literal) == 0)
        {
            pos_ += len;
            return true;
        }
        return false;
    }

    JsonValue parseValue()
    {
        skipWhitespace();
        char c = peek();
        switch (c)
        {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': return JsonValue(parseString());
        case 't':
            if (consumeLiteral("true")) return JsonValue(true);
            fail("invalid literal");
        case 'f':
            if (consumeLiteral("false")) return JsonValue(false);
            fail("invalid literal");
        case 'n':
            if (consumeLiteral("null")) return JsonValue();
            fail("invalid literal");
        default:
            if (c == '-' || (c >= '0' && c <= '9'))
            {
                return parseNumber();
            }
            fail("unexpected character");
        }
    }

    JsonValue parseObject()
    {
        expect('{');
        JsonValue obj = JsonValue::makeObject();
        skipWhitespace();
        if (peek() == '}')
        {
            next();
            return obj;
        }
        while (true)
        {
            skipWhitespace();
            std::string key = parseString();
            skipWhitespace();
            expect(':');
            JsonValue value = parseValue();
            obj[key] = value;
            skipWhitespace();
            char c = next();
            if (c == ',')
            {
                continue;
            }
            if (c == '}')
            {
                break;
            }
            fail("expected ',' or '}'");
        }
        return obj;
    }

    JsonValue parseArray()
    {
        expect('[');
        JsonValue arr = JsonValue::makeArray();
        skipWhitespace();
        if (peek() == ']')
        {
            next();
            return arr;
        }
        while (true)
        {
            JsonValue value = parseValue();
            arr.push_back(value);
            skipWhitespace();
            char c = next();
            if (c == ',')
            {
                continue;
            }
            if (c == ']')
            {
                break;
            }
            fail("expected ',' or ']'");
        }
        return arr;
    }

    std::string parseString()
    {
        expect('"');
        std::string out;
        while (true)
        {
            char c = next();
            if (c == '"')
            {
                break;
            }
            if (c == '\\')
            {
                char esc = next();
                switch (esc)
                {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'u':
                {
                    unsigned int code = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        char h = next();
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= (h - '0');
                        else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                        else fail("invalid unicode escape");
                    }
                    // Minimal handling: encode as UTF-8 for code points < 0x10000.
                    if (code < 0x80)
                    {
                        out += static_cast<char>(code);
                    }
                    else if (code < 0x800)
                    {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    else
                    {
                        out += static_cast<char>(0xE0 | (code >> 12));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default:
                    fail("invalid escape sequence");
                }
            }
            else
            {
                out += c;
            }
        }
        return out;
    }

    JsonValue parseNumber()
    {
        size_t start = pos_;
        if (peek() == '-')
        {
            ++pos_;
        }
        while (pos_ < text_.size() && isdigit(static_cast<unsigned char>(text_[pos_])))
        {
            ++pos_;
        }
        if (pos_ < text_.size() && text_[pos_] == '.')
        {
            ++pos_;
            while (pos_ < text_.size() && isdigit(static_cast<unsigned char>(text_[pos_])))
            {
                ++pos_;
            }
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E'))
        {
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-'))
            {
                ++pos_;
            }
            while (pos_ < text_.size() && isdigit(static_cast<unsigned char>(text_[pos_])))
            {
                ++pos_;
            }
        }
        std::string numText = text_.substr(start, pos_ - start);
        if (numText.empty() || numText == "-")
        {
            fail("invalid number");
        }
        return JsonValue(std::stod(numText));
    }
};

} // namespace

JsonValue JsonValue::parse(const std::string& text)
{
    JsonParser parser(text);
    return parser.parse();
}

JsonValue JsonValue::parseFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        throw JsonParseError("could not open file for reading: " + path);
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return parse(buffer.str());
}

void JsonValue::saveToFile(const std::string& path) const
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        throw JsonParseError("could not open file for writing: " + path);
    }
    out << serialize();
}
