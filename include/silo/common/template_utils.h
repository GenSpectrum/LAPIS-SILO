#pragma once

#include <cstddef>

namespace silo {

template <
   size_t N,
   template <typename, typename>
   typename Container,
   typename ContainerArg,
   typename Base>
struct NestedContainer {
   using type =
      Container<ContainerArg, typename NestedContainer<N - 1, Container, ContainerArg, Base>::type>;
};

template <template <typename, typename> typename Container, typename ContainerArg, typename Base>
struct NestedContainer<1, Container, ContainerArg, Base> {
   using type = Container<ContainerArg, Base>;
};

}  // namespace silo
