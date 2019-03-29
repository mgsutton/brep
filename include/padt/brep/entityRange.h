#ifndef PADT_BREP_ENTITY_RANGE_H
#define PADT_BREP_ENTITY_RANGE_H

namespace padt::brep {
template <typename ContainerT, typename IterT,
          IterT (ContainerT::*begin_fn)() const,
          IterT (ContainerT::*end_fn)() const>
class EntityRange {
 public:
  typedef IterT iterator;
  typedef IterT const_iterator;
  EntityRange(ContainerT &c) : container_(c) {}
  IterT begin() const { return (container_.*begin_fn)(); }
  IterT end() const { return (container_.*end_fn)(); }

 private:
  ContainerT &container_;
};
}
#endif


