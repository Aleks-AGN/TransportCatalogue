#include "json_builder.h"

namespace json {
    using namespace std::literals;
    
    Builder::KeyContext::KeyContext(Builder& builder) 
        : builder_(builder) {
    }
    Builder::DictItemContext Builder::KeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(value));
    }
    Builder::DictItemContext Builder::KeyContext::StartDict() {
        return builder_.StartDict();
    }
    Builder::ArrayItemContext Builder::KeyContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::DictItemContext::DictItemContext(Builder& builder) 
        : builder_(builder) {
    }
    Builder::KeyContext Builder::DictItemContext::Key(const std::string& key) {
        return builder_.Key(key);
    }
    Builder& Builder::DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
        : builder_(builder) {
    }
    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(value));
    }
    Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }
    Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& Builder::ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::Builder() {
        nodes_stack_.push_back(&root_);
    }

    Node* Builder::AddItem(Node::Value value) {
        Node node;
        node.GetValue() = move(value);

        Node* node_ptr = nodes_stack_.back();

        if (node_ptr->IsNull()) {
            *node_ptr = move(node);
            nodes_stack_.pop_back();
            return &root_;
        } else if (node_ptr->IsDict()) {
            if (!key_) {
                throw std::logic_error("Ошибка: добавление элемента в словарь без ключа."s);
            }
            Dict& dict = const_cast<Dict&>(node_ptr->AsDict());
            const auto ptr = dict.emplace(key_.value(), node);
            key_ = std::nullopt;
            return &ptr.first->second;
        } else if (node_ptr->IsArray()) {
            if (key_) {
                throw std::logic_error("Ошибка: добавление ключа в массив."s);
            }
            Array& array = const_cast<Array&>(node_ptr->AsArray());
            array.emplace_back(node);
            return &array.back();
        }
        return nullptr;
    }
    
    Builder::KeyContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || root_.IsNull()) {
            throw std::logic_error("Ошибка: вызов любого метода, кроме Build(), при готовом объекте."s);
        }
        if (key_ || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Ошибка: вызов метода Key() снаружи словаря или сразу после другого Key."s);
        }
        key_ = move(key);

        return KeyContext(*this);
    }

    Builder& Builder::Value(Node::Value value) {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Ошибка: вызов любого метода, кроме Build(), при готовом объекте."s);
        }
        AddItem(value);

        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Ошибка: dызов любого метода, кроме Build(), при готовом объекте."s);
        }
        nodes_stack_.push_back(AddItem(Dict()));

        return DictItemContext(*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Ошибка: вызов любого метода, кроме Build(), при готовом объекте."s);
        }
        nodes_stack_.push_back(AddItem(Array()));

        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Ошибка: Метод EndDict() завершает не словарь."s);
        }
        nodes_stack_.pop_back();

        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Ошибка: Метод EndArray() завершает не массив."s);
        }
        nodes_stack_.pop_back();

        return *this;
    }

    Node& Builder::Build() {
        if (!nodes_stack_.empty() || root_.IsNull()) {
            throw std::logic_error("Вызов метода Build() при неготовом описываемом объекте."s);
        }
        return root_;
    }

}  // namespace json
