#ifndef DECOTERM_SYNC_HPP
#define DECOTERM_SYNC_HPP

#include "output.hpp"

#include <syncstream>

namespace deco {

/// @brief An `OStream` that buffers output using `std::osyncstream`.
///
/// @details
/// For example:
///
/// ```cpp
/// OSyncStream out(std::cout);
/// out << styled("synced", bold);
/// out.emit();
/// ```
/// The above is equivalent to:
///
/// ```cpp
/// std::osyncstream oss(std::cout);
/// OStream out(oss);
/// out << styled("synced", bold);
/// oss.emit();
/// ```
/// @warning Similar to `std::osyncstream`, `OSyncStream` itself is not safe for
/// concurrent access. The same `OSyncStream` object must not be used by
/// multiple threads simultaneously, as doing so may cause a data race.
class OSyncStream : public OStream {
  public:
    explicit OSyncStream(std::ostream& ostream)
        : OStream(ostream),
          osyncstream_(ostream) {
        this->switch_ostream(osyncstream_);
    }

    explicit OSyncStream(const OutputConfig& config, std::ostream& ostream)
        : OStream(config, ostream),
          osyncstream_(ostream) {
        this->switch_ostream(osyncstream_);
    }

    /// Emits the buffered output to the wrapped stream.
    void emit() { osyncstream_.emit(); }

  private:
    std::osyncstream osyncstream_;
};

} // namespace deco

#endif // !DECOTERM_SYNC_HPP
