#ifndef PADT_BREP_HANDLE_H
#define PADT_BREP_HANDLE_H
#include <ostream>
#include <variant>

namespace padt::brep {
class BaseHandle {
 public:
  explicit BaseHandle(int h = -1) : h_(h) {}
  bool isValid() const { return h_ != -1; }
  int idx() const { return h_; }
  void reset() { h_ = -1; }
  void invalidate() { reset(); }
  bool operator==(const BaseHandle& rhs) const { return h_ == rhs.h_; }
  bool operator!=(const BaseHandle& rhs) const { return h_ != rhs.h_; }
  bool operator<(const BaseHandle& rhs) const { return h_ < rhs.h_; }

 private:
  int h_;
};

template <typename T>
struct HandleHash {
  size_t operator()(const T& h) const { return h.idx(); }
};

inline std::ostream& operator<<(std::ostream& os, const BaseHandle& h) {
  return (os << h.idx());
}

class AssemblyHandle : public BaseHandle {
 public:
  explicit AssemblyHandle(int h = -1) : BaseHandle(h) {}
};

class PartHandle : public BaseHandle {
 public:
  explicit PartHandle(int h = -1) : BaseHandle(h) {}
};

class BodyHandle : public BaseHandle {
 public:
  explicit BodyHandle(int h = -1) : BaseHandle(h) {}
};

class FaceHandle : public BaseHandle {
 public:
  explicit FaceHandle(int h = -1) : BaseHandle(h) {}
};

class EdgeHandle : public BaseHandle {
 public:
  explicit EdgeHandle(int h = -1) : BaseHandle(h) {}
};

class VertexHandle : public BaseHandle {
 public:
  explicit VertexHandle(int h = -1) : BaseHandle(h) {}
};

class LoopHandle : public BaseHandle {
 public:
  explicit LoopHandle(int h = -1) : BaseHandle(h) {}
};

class ShellHandle : public BaseHandle {
 public:
  explicit ShellHandle(int h = -1) : BaseHandle(h) {}
};

typedef std::variant<PartHandle, BodyHandle, FaceHandle, EdgeHandle,
                     VertexHandle>
    EntityHandle;
}  // namespace padt::brep
#endif