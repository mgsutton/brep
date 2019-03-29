#ifndef PADT_BREP_H
#define PADT_BREP_H
// PADT BRep
#include <Eigen/Dense>
#include <boost/range/adaptor/map.hpp>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "assembly.h"
#include "body.h"
#include "brep.pb.h"
#include "edge.h"
#include "entity.h"
#include "entityRange.h"
#include "face.h"
#include "handle.h"
#include "part.h"
#include "vertex.h"

namespace padt::brep {

class BRep {
 public:
  // Default constructors, move constructors and assignment operators
  BRep() = default;
  ~BRep() = default;
  BRep(const BRep &) = default;
  BRep(BRep &&) = default;
  BRep &operator=(const BRep &) = default;
  BRep &operator=(BRep &&) = default;

  // Construction functions
  void reset();
  bool buildBRepFromFile(const std::string &filename);
  bool buildBRepFromEntityStream(
      const std::vector<padt::brep::proto::BRepEntity> &entities);

  // Bounding box functions
  Eigen::AlignedBox3d boundingBox(const AssemblyHandle &h) const;
  Eigen::AlignedBox3d boundingBox(const PartHandle &h) const;
  Eigen::AlignedBox3d boundingBox(const BodyHandle &h) const;
  Eigen::AlignedBox3d boundingBox(const FaceHandle &h) const;
  Eigen::AlignedBox3d boundingBox(const EdgeHandle &h) const;

  // Access functions
  auto assemblies() const { return boost::adaptors::keys(Assemblies_); }
  auto parts() const { return boost::adaptors::keys(Parts_); }
  auto bodies() const { return boost::adaptors::keys(Bodies_); }
  auto faces() const { return boost::adaptors::keys(Faces_); }
  auto edges() const { return boost::adaptors::keys(Edges_); }

  const VertexHandle startVertex(const EdgeHandle &h) const {
    if (Edges_.count(h) > 0) {
      return Edges_.at(h).startVertex_;
    }
    return VertexHandle(-1);
  }

  const VertexHandle endVertex(const EdgeHandle &h) const {
    if (Edges_.count(h) > 0) {
      return Edges_.at(h).endVertex_;
    }
    return VertexHandle(-1);
  }

  const std::vector<EdgeHandle> &faceEdges(const FaceHandle &h) const {
    if (Faces_.count(h) > 0) {
      return Faces_.at(h).Edges_;
    }
    return EmptyEdges_;
  }
  const std::vector<FaceHandle> &bodyFaces(const BodyHandle &h) const {
    if (Bodies_.count(h) > 0) {
      return Bodies_.at(h).Faces_;
    }
    return EmptyFaces_;
  }
  const std::vector<BodyHandle> &partBodies(const PartHandle &h) const {
    if (Parts_.count(h) > 0) {
      return Parts_.at(h).Bodies_;
    }
    return EmptyBodies_;
  }

  std::tuple<const Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>,
             const Eigen::Block<const Eigen::Matrix<int, Eigen::Dynamic, 3>>>
  faceGeometry(const FaceHandle &h) const {
    if (Faces_.count(h) == 0) {
      return std::make_tuple(
          Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>(
              EmptyV_, 0, 0, 0, 0),
          Eigen::Block<const Eigen::Matrix<int, Eigen::Dynamic, 3>>(EmptyF_, 0,
                                                                    0, 0, 0));
    }
    const auto &faceData = Faces_.at(h);
    Eigen::Index startV = faceData.PointStart_;
    Eigen::Index sizeV = faceData.PointSize_;
    Eigen::Index startF = faceData.FacetStart_;
    Eigen::Index sizeF = faceData.FacetSize_;
    return std::make_tuple(
        Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>(
            V_, startV, 0, sizeV, 3),
        Eigen::Block<const Eigen::Matrix<int, Eigen::Dynamic, 3>>(F_, startF, 0,
                                                                  sizeF, 3));
  }

  const Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>
  edgeGeometry(const EdgeHandle &h) const {
    if (Edges_.count(h) == 0) {
      return Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>(
          EmptyV_, 0, 0, 0, 0);
    }
    const auto &edgeData = Edges_.at(h);
    Eigen::Index startV = edgeData.PointStart_;
    Eigen::Index sizeV = edgeData.PointSize_;
    return Eigen::Block<const Eigen::Matrix<double, Eigen::Dynamic, 3>>(
        V_, startV, 0, sizeV, 3);
  }

 private:
  void resetGeneratedHandleId(
      const std::vector<padt::brep::proto::BRepEntity> &entities);
  LoopHandle generateNewLoopHandle() {
    return LoopHandle(++GeneratedHandleId_);
  }
  ShellHandle generateNewShellHandle() {
    return ShellHandle(++GeneratedHandleId_);
  }
  AssemblyHandle generateNewAssemblyHandle() {
    return AssemblyHandle(++GeneratedHandleId_);
  }

  bool addBRepEntity(const padt::brep::proto::BRepEntity &entity);
  bool addAssembly(const padt::brep::proto::Assembly &assembly);
  bool addPart(const padt::brep::proto::Part &part);
  bool addBody(const padt::brep::proto::Body &body);
  bool addFace(const padt::brep::proto::Face &face);
  bool addEdge(const padt::brep::proto::Edge &edge);
  bool addVertex(const padt::brep::proto::Vertex &vertex);
  bool buildBottomUpTopology();

 private:
  // Geometry
  Eigen::Matrix<double, Eigen::Dynamic, 3> V_;
  Eigen::Matrix<int, Eigen::Dynamic, 3> F_;
  Eigen::Matrix<double, Eigen::Dynamic, 2> FaceParams_;
  Eigen::VectorXd EdgeParams_;
  Eigen::Matrix<double, Eigen::Dynamic, 3> EmptyV_;
  Eigen::Matrix<int, Eigen::Dynamic, 3> EmptyF_;
  // Topology structures
  struct AssemblyData {
    std::vector<PartHandle> Parts_;
  };

  struct PartData {
    std::vector<BodyHandle> Bodies_;
  };

  struct BodyData {
    std::vector<FaceHandle> Faces_;
    std::vector<ShellHandle> Shells_;
    std::vector<EdgeHandle> Edges_;
    std::vector<VertexHandle> Vertices_;
  };

  struct FaceData {
    FaceData()
        : PointStart_(-1),
          PointSize_(-1),
          FacetStart_(-1),
          FacetSize_(-1),
          ParamStart_(-1),
          ParamSize_(-1) {}
    std::vector<EdgeHandle> Edges_;
    std::vector<LoopHandle> Loops_;
    std::vector<BodyHandle> Bodies_;
    Eigen::Index PointStart_;
    Eigen::Index PointSize_;
    Eigen::Index FacetStart_;
    Eigen::Index FacetSize_;
    Eigen::Index ParamStart_;
    Eigen::Index ParamSize_;
  };

  struct EdgeData {
    EdgeData()
        : PointStart_(-1), PointSize_(-1), ParamStart_(-1), ParamSize_(-1) {}
    VertexHandle startVertex_;
    VertexHandle endVertex_;
    std::vector<FaceHandle> Faces_;
    Eigen::Index PointStart_;
    Eigen::Index PointSize_;
    Eigen::Index ParamStart_;
    Eigen::Index ParamSize_;
  };

  struct VertexData {
    VertexData() : PointIndex_(-1) {}
    Eigen::Index PointIndex_;
    std::vector<EdgeHandle> Edges_;
  };

  struct LoopData {
    std::vector<EdgeHandle> Edges_;
  };

  struct ShellData {
    std::vector<FaceHandle> Faces_;
  };
  // Topology storage
  std::unordered_map<AssemblyHandle, AssemblyData, HandleHash<AssemblyHandle>>
      Assemblies_;
  std::unordered_map<PartHandle, PartData, HandleHash<PartHandle>> Parts_;
  std::unordered_map<BodyHandle, BodyData, HandleHash<BodyHandle>> Bodies_;
  std::unordered_map<FaceHandle, FaceData, HandleHash<FaceHandle>> Faces_;
  std::unordered_map<EdgeHandle, EdgeData, HandleHash<EdgeHandle>> Edges_;
  std::unordered_map<VertexHandle, VertexData, HandleHash<VertexHandle>>
      Vertices_;
  std::unordered_map<LoopHandle, LoopData, HandleHash<LoopHandle>> Loops_;
  std::unordered_map<ShellHandle, ShellData, HandleHash<ShellHandle>> Shells_;
  // Empty ranges of entities
  std::vector<AssemblyHandle> EmptyAssemblies_;
  std::vector<PartHandle> EmptyParts_;
  std::vector<BodyHandle> EmptyBodies_;
  std::vector<FaceHandle> EmptyFaces_;
  std::vector<EdgeHandle> EmptyEdges_;
  std::vector<VertexHandle> EmptyVertices_;
  std::vector<LoopHandle> EmptyLoops_;
  std::vector<ShellHandle> EmptyShells_;

  int GeneratedHandleId_;

  // Bounding Box cached values
  mutable std::unordered_map<AssemblyHandle, Eigen::AlignedBox3d,
                             HandleHash<AssemblyHandle>>
      AssemblyBBoxCache_;
  mutable std::unordered_map<PartHandle, Eigen::AlignedBox3d,
                             HandleHash<PartHandle>>
      PartBBoxCache_;
  mutable std::unordered_map<BodyHandle, Eigen::AlignedBox3d,
                             HandleHash<BodyHandle>>
      BodyBBoxCache_;
  mutable std::unordered_map<FaceHandle, Eigen::AlignedBox3d,
                             HandleHash<FaceHandle>>
      FaceBBoxCache_;
  mutable std::unordered_map<EdgeHandle, Eigen::AlignedBox3d,
                             HandleHash<EdgeHandle>>
      EdgeBBoxCache_;
};  // namespace padt::brep
}  // namespace padt::brep

#endif