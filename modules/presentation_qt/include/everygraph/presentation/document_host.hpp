#pragma once

#include <QMainWindow>
#include <QRect>
#include <QString>
#include <QWidget>

#include <map>
#include <string>
#include <vector>

class QAction;
class QCloseEvent;
class QTabWidget;

namespace everygraph::presentation {

struct DocumentDescriptor {
    std::string document_id;
    QString title;
    QString kind;
};

// A floated document. Closing the window docks the document back instead of
// discarding it, so a prototype user cannot lose work by closing the float.
class DetachedDocumentWindow : public QMainWindow {
    Q_OBJECT

public:
    DetachedDocumentWindow(DocumentDescriptor descriptor, QWidget* content,
                           QWidget* parent = nullptr);

    [[nodiscard]] const DocumentDescriptor& descriptor() const noexcept;
    [[nodiscard]] QWidget* content() const noexcept;
    void request_dock_back();

signals:
    void dock_back_requested(const QString& document_id);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    DocumentDescriptor descriptor_;
    QWidget* content_{nullptr};
    QAction* dock_action_{nullptr};
};

// Central tabbed document host. Tabs and detached windows are UI state; the
// document content is never mutated here. The host takes ownership of each
// content widget and deletes it when the document closes.
class DocumentHost : public QWidget {
    Q_OBJECT

public:
    explicit DocumentHost(QWidget* parent = nullptr);
    ~DocumentHost() override;

    [[nodiscard]] bool open_document(DocumentDescriptor descriptor, QWidget* content);
    [[nodiscard]] bool close_document(const std::string& document_id);
    [[nodiscard]] bool detach_document(const std::string& document_id);
    [[nodiscard]] bool dock_document(const std::string& document_id);
    void restore_detached_windows(const std::vector<QRect>& available_areas);

    [[nodiscard]] int document_count() const noexcept;
    [[nodiscard]] int tab_count() const noexcept;
    [[nodiscard]] bool contains(const std::string& document_id) const;
    [[nodiscard]] bool is_detached(const std::string& document_id) const;
    [[nodiscard]] QWidget* content(const std::string& document_id) const;
    [[nodiscard]] QString current_document_id() const;
    [[nodiscard]] QTabWidget* tab_widget() const noexcept;
    [[nodiscard]] DetachedDocumentWindow* detached_window(const std::string& document_id) const;
    [[nodiscard]] QString title(const std::string& document_id) const;

signals:
    void document_opened(const QString& document_id);
    void document_closed(const QString& document_id);
    void document_detached(const QString& document_id);
    void document_docked(const QString& document_id);
    void current_document_changed(const QString& document_id);

private:
    struct Entry {
        DocumentDescriptor descriptor;
        QWidget* content{nullptr};
        DetachedDocumentWindow* detached{nullptr};
    };

    [[nodiscard]] std::map<std::string, Entry>::iterator find_entry(const std::string& document_id);
    [[nodiscard]] std::map<std::string, Entry>::const_iterator find_entry(
        const std::string& document_id) const;
    [[nodiscard]] std::string document_id_for_content(const QWidget* content) const;
    void attach_tab(Entry& entry);
    void handle_dock_back(const QString& document_id);

    QTabWidget* tabs_{nullptr};
    std::map<std::string, Entry> documents_;
};

}  // namespace everygraph::presentation
