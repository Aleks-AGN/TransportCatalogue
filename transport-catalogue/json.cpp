#include "json.h"

using namespace std::literals;

namespace json {

namespace {

Node LoadNode(std::istream& input);

Node LoadNull(std::istream& input) {
    std::string result;

    while (std::isalpha((input.peek()))) {
        result.push_back(input.get());
    }
    if (result == "null"s) {
        return Node{nullptr};
    }
    throw ParsingError("Null parsing error"s);
}

Node LoadBool(std::istream& input) {
    std::string result;

    while (std::isalpha((input.peek()))) {
        result.push_back(input.get());
    }
    if (result == "true"s) {
        return Node{true};
    } else if (result == "false"s) {
        return Node{false};
    }
    throw ParsingError("Bool parsing error"s);
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;
    
    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };
    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };
    
    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }
    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }
    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }
    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node{std::stoi(parsed_num)};
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node{std::stod(parsed_num)};
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    auto pos = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string result;
    while (true) {
        if (pos == end) { // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error"s);
        }
        const char ch = *pos;
        if (ch == '"') { // Встретили закрывающую кавычку
            ++pos;
            break;
        } else if (ch == '\\') { // Встретили начало escape-последовательности
            ++pos;
            if (pos == end) { // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            const char escaped_symbol = *(pos);
            switch (escaped_symbol) { // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                case 'n':
                    result.push_back('\n');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case '"':
                    result.push_back('"');
                    break;
                case '\\':
                    result.push_back('\\');
                    break;
                default: // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_symbol);
            }
        } else if (ch == '\n' || ch == '\r') { // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else { // Просто считываем очередной символ и помещаем его в результирующую строку
            result.push_back(ch);
        }
        ++pos;
    }
    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict result;
	char ch;
	while (input >> ch) {
		if (ch == '}') {
			return Node(std::move(result));
		}
		if (ch == ',') {
			input >> ch;
		}
		std::string key = LoadString(input).AsString();
		input >> ch;
		result.insert({ move(key), LoadNode(input) });
	}
	throw ParsingError("Map parsing error"s);
}

Node LoadArray(std::istream& input) {
    Array result;
    char ch;
	while (input >> ch) {
		if (ch == ']') {
			return Node(std::move(result));
		}
		if (ch != ',') {
			input.putback(ch);
		}
		result.push_back(LoadNode(input));
	}
	throw ParsingError("Array parsing error"s);
}


Node LoadNode(std::istream& input) {
    char ch;
    if (!(input >> ch)) {
        throw ParsingError("Node parsing error"s);
    }

    if (ch == 'n') {
        input.putback(ch);
        return LoadNull(input);
    } else if (ch == 't' || ch == 'f') {
        input.putback(ch);
        return LoadBool(input);
    } else if (ch == '[') {
        return LoadArray(input);
    } else if (ch == '{') {
        return LoadDict(input);
    } else if (ch == '"') {
        return LoadString(input);
    } else {
        input.putback(ch);
        return LoadNumber(input);
    }
}

// Контекст вывода, хранит ссылку на поток вывода и текущий отступ
struct PrintContext {
    PrintContext(std::ostream& out)
        : out(out) {
    }

    PrintContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }

    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
};

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(bool value, std::ostream& out) {
    out << (value ? "true"s : "false"s);
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << (value ? "true"s : "false"s);
}

void PrintValue(int value, std::ostream& out) {
    out << value;
}

void PrintValue(int value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(double value, std::ostream& out) {
    out << value;
}

void PrintValue(double value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(const std::string& value, std::ostream& out) {
    out << '"';
    for (const char ch : value) {
        switch (ch) {
            case '\r':
                out << "\\r"s;
                break;
            case '\n':
                out << "\\n"s;
                break;
            case '\t':
                out << "\t"s;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                out << '\\';
                [[fallthrough]];
            default:
                out << ch;
                break;
        }
    }
    out << '"';
}

void PrintValue(const std::string& value, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << '"';
    for (const char ch : value) {
        switch (ch) {
            case '\r':
                ctx.out << "\\r"s;
                break;
            case '\n':
                ctx.out << "\\n"s;
                break;
            case '\t':
                ctx.out << "\t"s;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                ctx.out << '\\';
                [[fallthrough]];
            default:
                ctx.out << ch;
                break;
        }
    }
    ctx.out << '"';
}

void PrintValue(const Dict& map, std::ostream& out);
void PrintValue(const Dict& map, const PrintContext& ctx);

void PrintValue(const Array& array, std::ostream& out) {
    out << '[';
    bool isFirst = true;
    for (const auto& value : array) {
        if (!isFirst) {
            out << ", "s;
        }
        isFirst = false;
        std::visit([&out](const auto& value){ PrintValue(value, out); },
            value.GetValue());
    }
    out << ']';
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    //ctx.PrintIndent();
    ctx.out << '[' << std::endl;
    bool isFirst = true;
    for (const auto& value : array) {
        if (!isFirst) {
            ctx.out << ',' << std::endl;
        }
        isFirst = false;
        std::visit([&ctx](const auto& value){ PrintValue(value, ctx.Indented()); },
            value.GetValue());
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << ']';
}

void PrintValue(const Dict& map, std::ostream& out) {
    out << '{';
    bool isFirst = true;
    for (const auto& [key, value] : map) {
        if (!isFirst) {
            out << ", "s;
        }
        isFirst = false;
        std::visit([&out](const auto& value){ PrintValue(value, out); },
            Node{key}.GetValue());
        out << ':';
        std::visit([&out](const auto& value){ PrintValue(value, out); },
            value.GetValue());
    }
    out << '}';
}

void PrintValue(const Dict& map, const PrintContext& ctx) {
    ctx.PrintIndent();
    ctx.out << '{' << std::endl;
    bool isFirst = true;
    for (const auto& [key, value] : map) {
        if (!isFirst) {
            ctx.out << ',' << std::endl;;
        }
        isFirst = false;
        std::visit([&ctx](const auto& value){ PrintValue(value, ctx.Indented()); },
            Node{key}.GetValue());
        ctx.out << ": "s;
        std::visit([&ctx](const auto& value){ PrintValue(value, ctx.Indented()); },
            value.GetValue());
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << '}';
}

}  // namespace

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

const Array& Node::AsArray() const {
    if (auto* value = std::get_if<Array>(this)) {
        return *value;
    }
    throw std::logic_error("Error: Impossible to parse node as Array"s);
}

const Dict& Node::AsMap() const {
    if (auto* value = std::get_if<Dict>(this)) {
        return *value;
    }
    throw std::logic_error("Error: Impossible to parse node as Map"s);
}

bool Node::AsBool() const {
    if (auto* value = std::get_if<bool>(this)) {
        return *value;
    }
    throw std::logic_error("Error: Impossible to parse node as Boolean"s);
}

int Node::AsInt() const {
    if (auto* value = std::get_if<int>(this)) {
        return *value;
    }
    throw std::logic_error("Error: Impossible to parse node as Integer"s);
}

double Node::AsDouble() const { 
    if (auto* value = std::get_if<double>(this)) {
        return *value;
    }
    if (auto* value = std::get_if<int>(this)) {
        return static_cast<double>(*value);
    }
    throw std::logic_error("Error: Impossible to parse node as Double"s);
}

const std::string& Node::AsString() const {
    if (auto* value = std::get_if<std::string>(this)) {
        return *value;
    }
    throw std::logic_error("Error: Impossible to parse node as String"s);
}

const Node::Value& Node::GetValue() const {
    return *this;
}

bool Node::operator==(const Node& other) const {
    return GetValue() == other.GetValue();
}

bool Node::operator!=(const Node& other) const {
    return GetValue() != other.GetValue();
}


Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return GetRoot() == other.GetRoot();
}

bool Document::operator!=(const Document& other) const {
    return GetRoot() != other.GetRoot();
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& out) {
    std::visit([&out](const auto& value){ PrintValue(value, out); },
        doc.GetRoot().GetValue());
}

void PrintWithContext(const Document& doc, std::ostream& out) {
    // PrintContext ctx(out, 2, 2);
    PrintContext ctx(out);
    std::visit([&ctx](const auto& value){ PrintValue(value, ctx); },
        doc.GetRoot().GetValue());
}

}  // namespace json
