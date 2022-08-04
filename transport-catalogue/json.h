#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

// class Node {
class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    // Делаем доступными все конструкторы родительского класса variant
    using variant::variant;
    using Value = variant;

    bool IsNull() const;
	bool IsArray() const;
	bool IsMap() const;
	bool IsBool() const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsPureDouble() const;
	bool IsString() const;
 
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;

    const Value& GetValue() const;
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

void PrintWithContext(const Document& doc, std::ostream& output);

}  // namespace json
