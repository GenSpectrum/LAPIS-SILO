#ifndef SILO_TEMPLATE_UTILS_H
#define SILO_TEMPLATE_UTILS_H

#include <cstddef>

namespace silo {

template <size_t N, template <typename> typename Container, typename Base>
struct NestedContainer {
   using type = Container<typename NestedContainer<N - 1, Container, Base>::type>;
};

template <template <typename> typename Container, typename Base>
struct NestedContainer<1, Container, Base> {
   using type = Container<Base>;
};

}  // namespace silo

#endif  // SILO_TEMPLATE_UTILS_H
