#pragma once

#include <QString>
#include <QWidget>

class QLabel;

namespace everygraph::application {
struct ObjectState;
}

namespace everygraph::presentation {

// Read-only properties dock contents. Values come from the kernel snapshot;
// the panel offers no editor and never writes to an object.
class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);

    void show_object(const application::ObjectState& object);
    void clear();

    [[nodiscard]] QString object_id_text() const;
    [[nodiscard]] QString object_type_text() const;
    [[nodiscard]] QString object_revision_text() const;
    [[nodiscard]] QString properties_text() const;

private:
    QLabel* object_id_{nullptr};
    QLabel* object_type_{nullptr};
    QLabel* object_revision_{nullptr};
    QLabel* properties_{nullptr};
};

}  // namespace everygraph::presentation
