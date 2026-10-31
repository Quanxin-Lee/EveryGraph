#include "everygraph/presentation/properties_panel.hpp"

#include <QFormLayout>
#include <QFont>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

#include <cstdint>
#include <string>
#include <variant>

#include "everygraph/application/document.hpp"
#include "everygraph/application/value.hpp"
#include "everygraph/foundation/ids.hpp"

namespace everygraph::presentation {
namespace {

template <typename... Visitors>
struct Overloaded : Visitors... {
    using Visitors::operator()...;
};

template <typename... Visitors>
Overloaded(Visitors...) -> Overloaded<Visitors...>;

QString format_value(const application::Value& value) {
    return std::visit(
        Overloaded{
            [](std::monostate) { return QStringLiteral("null"); },
            [](std::int64_t integer) { return QString::number(integer); },
            [](double number) { return QString::number(number, 'g', 17); },
            [](bool flag) { return flag ? QStringLiteral("true") : QStringLiteral("false"); },
            [](const std::string& text) { return QString::fromStdString(text); },
        },
        value);
}

QString format_properties(const application::ParameterMap& properties) {
    QStringList lines;
    for (const auto& [key, value] : properties) {
        lines.append(QStringLiteral("%1 = %2")
                         .arg(QString::fromStdString(key), format_value(value)));
    }
    return lines.isEmpty() ? QStringLiteral("(none)") : lines.join(QLatin1Char('\n'));
}

QString format_id(foundation::ObjectId id) {
    return id.is_valid() ? QString::number(static_cast<qulonglong>(id.value))
                         : QStringLiteral("(none)");
}

}  // namespace

PropertiesPanel::PropertiesPanel(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("propertiesPanel"));
    setAccessibleName(QStringLiteral("Properties panel"));
    setAccessibleDescription(
        QStringLiteral("Read-only properties of the selected project object."));
    setFocusPolicy(Qt::StrongFocus);
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel(QStringLiteral("Properties"), this);
    QFont title_font = title->font();
    title_font.setBold(true);
    title->setFont(title_font);
    layout->addWidget(title);

    auto* form = new QFormLayout();
    object_id_ = new QLabel(QStringLiteral("(none)"), this);
    object_type_ = new QLabel(QStringLiteral("(none)"), this);
    object_revision_ = new QLabel(QStringLiteral("(none)"), this);
    object_id_->setAccessibleName(QStringLiteral("Object ID value"));
    object_type_->setAccessibleName(QStringLiteral("Type value"));
    object_revision_->setAccessibleName(QStringLiteral("Revision value"));
    for (QLabel* label : {object_id_, object_type_, object_revision_}) {
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    form->addRow(QStringLiteral("Object ID"), object_id_);
    form->addRow(QStringLiteral("Type"), object_type_);
    form->addRow(QStringLiteral("Revision"), object_revision_);
    layout->addLayout(form);

    properties_ = new QLabel(QStringLiteral("(none)"), this);
    properties_->setObjectName(QStringLiteral("propertiesValues"));
    properties_->setAccessibleName(QStringLiteral("Property values"));
    properties_->setAccessibleDescription(
        QStringLiteral("Key and value pairs for the selected object."));
    properties_->setWordWrap(true);
    properties_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* properties_title = new QLabel(QStringLiteral("Properties"), this);
    layout->addWidget(properties_title);
    layout->addWidget(properties_);
    layout->addStretch(1);
}

void PropertiesPanel::show_object(const application::ObjectState& object) {
    object_id_->setText(format_id(object.id));
    object_type_->setText(QString::fromStdString(object.type));
    object_revision_->setText(QString::number(static_cast<qulonglong>(object.revision)));
    properties_->setText(format_properties(object.properties));
}

void PropertiesPanel::clear() {
    object_id_->setText(QStringLiteral("(none)"));
    object_type_->setText(QStringLiteral("(none)"));
    object_revision_->setText(QStringLiteral("(none)"));
    properties_->setText(QStringLiteral("No object selected"));
}

QString PropertiesPanel::object_id_text() const {
    return object_id_->text();
}

QString PropertiesPanel::object_type_text() const {
    return object_type_->text();
}

QString PropertiesPanel::object_revision_text() const {
    return object_revision_->text();
}

QString PropertiesPanel::properties_text() const {
    return properties_->text();
}

}  // namespace everygraph::presentation
