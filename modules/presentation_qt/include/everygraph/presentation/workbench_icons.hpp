#pragma once

#include <QIcon>

namespace everygraph::presentation {

enum class WorkbenchIcon {
    new_folder,
    new_dataset,
    move_object,
    find_object,
    open_document,
    undo,
    redo,
    detach_document,
    dock_document,
    project_tree,
    properties,
};

// Icons are painted at 1x, 2x and 4x from the same vector description, so the
// shell needs no bundled vendor image assets.
[[nodiscard]] QIcon make_workbench_icon(WorkbenchIcon icon);

}  // namespace everygraph::presentation
