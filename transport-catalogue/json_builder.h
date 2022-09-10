#pragma once

#include "json.h"
#include <optional>

namespace json {

class Builder {
public:
    class KeyContext;
    class DictItemContext;
    class ArrayItemContext;

    class KeyContext {
        friend class Builder;
    public:
        KeyContext(Builder& builder);
        DictItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    private:
        Builder& builder_;
    };

    class DictItemContext {
        friend class Builder;
    public:
        DictItemContext(Builder& builder);
        KeyContext Key(const std::string& key);
        Builder& EndDict();
    private:
        Builder& builder_;
    };

    class ArrayItemContext {
        friend class Builder;
    public:
        ArrayItemContext(Builder& builder);
        ArrayItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    private:
        Builder& builder_;
    };

    Builder();

    // При определении словаря задаёт строковое значение ключа для очередной
    // пары ключ-значение. Следующий вызов метода обязательно должен задавать
    // соответствующее этому ключу значение с помощью метода Value
    // или начинать его определение с помощью StartDict или StartArray.
    KeyContext Key(std::string key);

    // Задаёт значение, соответствующее ключу при определении словаря,
    // очередной элемент массива или, если вызвать сразу после конструктора
    // json::Builder, всё содержимое конструируемого JSON-объекта.
    // Может принимать как простой объект — число или строку — так и целый массив или словарь.
    // Node::Value — это синоним для базового класса Node,
    // шаблона variant с набором возможных типов-значений.
    Builder& Value(Node::Value value);

    // Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value.
    // Следующим вызовом обязательно должен быть Key или EndDict.
    DictItemContext StartDict();

    // Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value.
    // Следующим вызовом обязательно должен быть EndArray или любой,
    // задающий новое значение: Value, StartDict или StartArray.
    ArrayItemContext StartArray();

    // Завершает определение сложного значения-словаря.
    // Последним незавершённым вызовом Start* должен быть StartDict.
    Builder& EndDict();

    // Завершает определение сложного значения-массива.
    // Последним незавершённым вызовом Start* должен быть StartArray.
    Builder& EndArray();

    // Возвращает объект json::Node, содержащий JSON, описанный предыдущими
    // вызовами методов. К этому моменту для каждого Start* должен быть вызван
    // соответствующий End*. При этом сам объект должен быть определён,
    // то есть вызов json::Builder{}.Build() недопустим.
    Node& Build();

private:
    // сам конструируемый объект
    Node root_;

    // стек указателей на те вершины JSON, которые ещё не построены:
    // то есть текущее описываемое значение и цепочка его родителей.
    // Он поможет возвращаться в нужный контекст после вызова End-методов.
    std::vector<Node*> nodes_stack_;

    std::optional<std::string> key_;

    Node* AddItem(Node::Value value);
};

}  // namespace json
