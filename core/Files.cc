#include "Context.h"
#include <vector>

using namespace std;

namespace ruby_typer {
namespace core {

vector<int> findLineBreaks(const std::string &s) {
    vector<int> res;
    int i = 0;
    for (auto c : s) {
        if (c == '\n') {
            res.push_back(i);
        }
        i++;
    }
    return res;
}

File::File(std::string &&path_, std::string &&source_, Type source_type)
    : source_type(source_type), path_(path_), source_(source_), line_breaks(findLineBreaks(this->source_)) {}

File &FileRef::file(GlobalState &gs) const {
    return *(gs.files[_id]);
}

absl::string_view File::path() const {
    return this->path_;
}

absl::string_view File::source() const {
    return this->source_;
}
} // namespace core
} // namespace ruby_typer
