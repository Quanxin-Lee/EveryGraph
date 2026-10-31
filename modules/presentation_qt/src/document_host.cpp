#include "everygraph/presentation/document_host.hpp"

#include "everygraph/presentation/window_placement.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <utility>

namespace everygraph::presentation {

DetachedDocumentWindow::DetachedDocumentWindow(DocumentDescriptor descriptor,
                                               QWidget* content, QWidget* parent)
    : QMainWindow(parent), descriptor_(std::move(descriptor)), content_(content) {
    setObjectName(QStringLiteral("detachedDocumentWindow"));
    setWindowTitle(QStringLiteral("%1 - EveryGraph").arg(descriptor_.title));
    setAttribute(Qt::WA_DeleteOnClose, false);
    if (content_ != nullptr) {
        setCentralWidget(content_);
    }

    auto* toolbar = addToolBar(QStringLiteral("Document"));
    toolbar->setObjectName(QStringLiteral("detachedDocumentToolbar"));
    dock_action_ = toolbar->addAction(QStringLiteral("Dock Back"));
    dock_action_->setObjectName(QStringLiteral("dockBackAction"));
    dock_action_->setToolTip(QStringLiteral("Return this document to the workbench tabs"));
    connect(dock_action_, &QAction::triggered, this, &DetachedDocumentWindow::request_dock_back);

    resize(720, 480);
}

const DocumentDescriptor& DetachedDocumentWindow::descriptor() const noexcept {
    return descriptor_;
}

QWidget* DetachedDocumentWindow::content() const noexcept {
    return content_;
}

void DetachedDocumentWindow::request_dock_back() {
    hide();
    emit dock_back_requested(QString::fromStdString(descriptor_.document_id));
}

void DetachedDocumentWindow::closeEvent(QCloseEvent* event) {
    request_dock_back();
    event->ignore();
}

DocumentHost::DocumentHost(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("documentHost"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    tabs_ = new QTabWidget(this);
    tabs_->setObjectName(QStringLiteral("documentTabs"));
    tabs_->setDocumentMode(true);
    tabs_->setMovable(true);
    tabs_->setTabsClosable(true);
    layout->addWidget(tabs_);

    connect(tabs_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* page = tabs_->widget(index);
        const std::string document_id = document_id_for_content(page);
        if (!document_id.empty()) {
            (void)close_document(document_id);
        }
    });
    connect(tabs_, &QTabWidget::currentChanged, this, [this](int) {
        emit current_document_changed(current_document_id());
    });
}

DocumentHost::~DocumentHost() {
    const auto entries = std::move(documents_);
    documents_.clear();
    for (const auto& item : entries) {
        const Entry& entry = item.second;
        if (entry.detached != nullptr) {
            entry.detached->disconnect(this);
            QWidget* page = entry.detached->takeCentralWidget();
            delete entry.detached;
            delete page;
        } else if (entry.content != nullptr) {
            const int index = tabs_->indexOf(entry.content);
            if (index >= 0) {
                tabs_->removeTab(index);
            }
            delete entry.content;
        }
    }
}

bool DocumentHost::open_document(DocumentDescriptor descriptor, QWidget* content) {
    if (descriptor.document_id.empty() || content == nullptr ||
        documents_.contains(descriptor.document_id) || !document_id_for_content(content).empty()) {
        return false;
    }
    Entry entry;
    entry.descriptor = std::move(descriptor);
    entry.content = content;
    const std::string document_id = entry.descriptor.document_id;
    auto [iterator, inserted] = documents_.emplace(document_id, std::move(entry));
    if (!inserted) {
        return false;
    }
    attach_tab(iterator->second);
    tabs_->setCurrentWidget(content);
    emit document_opened(QString::fromStdString(document_id));
    return true;
}

bool DocumentHost::close_document(const std::string& document_id) {
    auto iterator = find_entry(document_id);
    if (iterator == documents_.end()) {
        return false;
    }
    Entry entry = std::move(iterator->second);
    documents_.erase(iterator);
    if (entry.detached != nullptr) {
        entry.detached->disconnect(this);
        QWidget* page = entry.detached->takeCentralWidget();
        entry.detached->hide();
        entry.detached->deleteLater();
        if (page != nullptr) {
            page->deleteLater();
        }
    } else {
        const int index = tabs_->indexOf(entry.content);
        if (index >= 0) {
            tabs_->removeTab(index);
        }
        if (entry.content != nullptr) {
            entry.content->deleteLater();
        }
    }
    emit document_closed(QString::fromStdString(document_id));
    return true;
}

bool DocumentHost::detach_document(const std::string& document_id) {
    auto iterator = find_entry(document_id);
    if (iterator == documents_.end() || iterator->second.detached != nullptr) {
        return false;
    }
    Entry& entry = iterator->second;
    const int index = tabs_->indexOf(entry.content);
    if (index >= 0) {
        tabs_->removeTab(index);
    }
    entry.content->setParent(nullptr);
    auto* window = new DetachedDocumentWindow(entry.descriptor, entry.content);
    connect(window, &DetachedDocumentWindow::dock_back_requested, this,
            &DocumentHost::handle_dock_back);
    entry.detached = window;
    window->show();
    emit document_detached(QString::fromStdString(document_id));
    return true;
}

bool DocumentHost::dock_document(const std::string& document_id) {
    auto iterator = find_entry(document_id);
    if (iterator == documents_.end() || iterator->second.detached == nullptr) {
        return false;
    }
    Entry& entry = iterator->second;
    DetachedDocumentWindow* window = entry.detached;
    QWidget* content = window->takeCentralWidget();
    window->disconnect(this);
    window->hide();
    window->deleteLater();
    entry.detached = nullptr;
    if (content != nullptr) {
        entry.content = content;
    }
    attach_tab(entry);
    tabs_->setCurrentWidget(entry.content);
    emit document_docked(QString::fromStdString(document_id));
    return true;
}

void DocumentHost::restore_detached_windows(const std::vector<QRect>& available_areas) {
    for (auto& item : documents_) {
        Entry& entry = item.second;
        if (entry.detached != nullptr) {
            WindowPlacement::ensure_visible(entry.detached, available_areas);
        }
    }
}

int DocumentHost::document_count() const noexcept {
    return static_cast<int>(documents_.size());
}

int DocumentHost::tab_count() const noexcept {
    return tabs_->count();
}

bool DocumentHost::contains(const std::string& document_id) const {
    return documents_.contains(document_id);
}

bool DocumentHost::is_detached(const std::string& document_id) const {
    const auto iterator = find_entry(document_id);
    return iterator != documents_.end() && iterator->second.detached != nullptr;
}

QWidget* DocumentHost::content(const std::string& document_id) const {
    const auto iterator = find_entry(document_id);
    return iterator == documents_.end() ? nullptr : iterator->second.content;
}

QString DocumentHost::current_document_id() const {
    return QString::fromStdString(document_id_for_content(tabs_->currentWidget()));
}

QTabWidget* DocumentHost::tab_widget() const noexcept {
    return tabs_;
}

DetachedDocumentWindow* DocumentHost::detached_window(const std::string& document_id) const {
    const auto iterator = find_entry(document_id);
    return iterator == documents_.end() ? nullptr : iterator->second.detached;
}

QString DocumentHost::title(const std::string& document_id) const {
    const auto iterator = find_entry(document_id);
    return iterator == documents_.end() ? QString() : iterator->second.descriptor.title;
}

std::map<std::string, DocumentHost::Entry>::iterator DocumentHost::find_entry(
    const std::string& document_id) {
    return documents_.find(document_id);
}

std::map<std::string, DocumentHost::Entry>::const_iterator DocumentHost::find_entry(
    const std::string& document_id) const {
    return documents_.find(document_id);
}

std::string DocumentHost::document_id_for_content(const QWidget* content) const {
    if (content == nullptr) {
        return {};
    }
    for (const auto& [document_id, entry] : documents_) {
        if (entry.content == content) {
            return document_id;
        }
    }
    return {};
}

void DocumentHost::attach_tab(Entry& entry) {
    const int index = tabs_->addTab(entry.content, entry.descriptor.title);
    tabs_->setTabToolTip(
        index, QStringLiteral("%1 (%2)").arg(entry.descriptor.title, entry.descriptor.kind));
    entry.detached = nullptr;
}

void DocumentHost::handle_dock_back(const QString& document_id) {
    (void)dock_document(document_id.toStdString());
}

}  // namespace everygraph::presentation
