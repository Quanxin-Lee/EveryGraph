#include "everygraph/foundation/ids.hpp"

namespace everygraph::foundation {

std::string to_string(ObjectId id) { return std::to_string(id.value); }

}  // namespace everygraph::foundation
