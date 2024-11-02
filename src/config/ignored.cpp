#include "config/ignored.h"

[[maybe_unused]] auto fmt::formatter<Ignored>::format(
   const Ignored& /* val */,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(ctx.out(), "Ignored");
}
