#include "everygraph/presentation/function_library_panel.hpp"

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace everygraph::presentation {

FunctionLibraryPanel::FunctionLibraryPanel(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("functionLibraryPanel"));
    setAccessibleName(QStringLiteral("Function library"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    list_ = new QListWidget(this);
    list_->setObjectName(QStringLiteral("functionLibraryList"));
    list_->setAccessibleName(QStringLiteral("Placeholder analysis functions"));
    list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    list_->addItems({QStringLiteral("Column Statistics"), QStringLiteral("FFT Spectrum"),
                     QStringLiteral("Gaussian Fit"), QStringLiteral("Linear Fit"),
                     QStringLiteral("Savitzky-Golay Smooth")});
    layout->addWidget(list_, 1);

    auto* note = new QLabel(
        QStringLiteral("Placeholder function library. Functions are listed for layout and "
                       "preference testing only; none executes in this prototype."),
        this);
    note->setWordWrap(true);
    layout->addWidget(note);
}

int FunctionLibraryPanel::function_count() const noexcept {
    return list_ == nullptr ? 0 : list_->count();
}

QStringList FunctionLibraryPanel::function_names() const {
    QStringList names;
    if (list_ == nullptr) {
        return names;
    }
    for (int row = 0; row < list_->count(); ++row) {
        if (const QListWidgetItem* item = list_->item(row); item != nullptr) {
            names.append(item->text());
        }
    }
    return names;
}

}  // namespace everygraph::presentation
