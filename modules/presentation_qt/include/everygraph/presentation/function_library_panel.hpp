#pragma once

#include <QStringList>
#include <QWidget>

class QListWidget;

namespace everygraph::presentation {

// Read-only placeholder function library. No function executes in this
// prototype; the panel exists so its visibility can be a persisted preference.
class FunctionLibraryPanel : public QWidget {
    Q_OBJECT

public:
    explicit FunctionLibraryPanel(QWidget* parent = nullptr);

    [[nodiscard]] int function_count() const noexcept;
    [[nodiscard]] QStringList function_names() const;

private:
    QListWidget* list_{nullptr};
};

}  // namespace everygraph::presentation
