#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}
// ---------- Color Prop --------------

void ColorPrinter::operator()(std::monostate) const {
    os << NoneColor;
}

void ColorPrinter::operator()(const std::string& color) const {
    os << color;
}

void ColorPrinter::operator()(Rgb color) const {
    os << "rgb("s
       << std::to_string(color.red) << ","s
       << std::to_string(color.green) << ","
       << std::to_string(color.blue) << ")"s;
}

void ColorPrinter::operator()(Rgba color) const {
    os << "rgba("s
       << std::to_string(color.red) << ","s
       << std::to_string(color.green) << ","s
       << std::to_string(color.blue) << ","s;
    os << color.opacity << ")"s;
}

std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit(ColorPrinter{os}, color);
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value) {
    switch (value) {
        case StrokeLineCap::BUTT:
            return os << "butt"s;
        case StrokeLineCap::ROUND:
            return os << "round"s;
        case StrokeLineCap::SQUARE:
            return os << "square"s;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value) {
    switch (value) {
        case StrokeLineJoin::ARCS:
            return os << "arcs"s;
        case StrokeLineJoin::BEVEL:
            return os << "bevel"s;
        case StrokeLineJoin::MITER:
            return os << "miter"s;
        case StrokeLineJoin::MITER_CLIP:
            return os << "miter-clip"s;
        case StrokeLineJoin::ROUND:
            return os << "round"s;
    }
    return os;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    bool isFirst = true;
    for (const auto& point : points_) {
        if (isFirst) {
            out << point.x << "," << point.y;
            isFirst = false;
        } else {
            out << " " << point.x << "," << point.y;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text --------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

std::string Text::PreprocessingData(const std::string& data) const {
    std::string result = data;

    for (const auto& [ch, str] : characters_to_change_) {
        size_t pos = 0;

        while (true) {
            pos = result.find(ch, pos);
            if (pos == std::string::npos) {
                break;
            }
            result.replace(pos, 1, str);
            pos = pos + str.size();
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">" << PreprocessingData(data_) << "</text>"sv;
}

// ---------- Document ----------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& object : objects_) {
        object->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg
