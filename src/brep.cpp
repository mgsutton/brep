/*
 MIT License
 Copyright (c) 2019 Matt Sutton
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 © 2019 GitHub, Inc.
*/

#include "brep.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <algorithm>
#include <fstream>
#include <numeric>
#include <vector>
#include "brep.pb.h"
#include "utility.h"

namespace padt::brep {

void BRep::reset() {
  Assemblies_.clear();
  Parts_.clear();
  Bodies_.clear();
  Faces_.clear();
  Edges_.clear();
  Vertices_.clear();
  Loops_.clear();
  Shells_.clear();

  AssemblyBBoxCache_.clear();
  PartBBoxCache_.clear();
  BodyBBoxCache_.clear();
  FaceBBoxCache_.clear();
  EdgeBBoxCache_.clear();
}


bool BRep::buildBRepFromFile(const std::string &fileName) {
  // See if we can open the file
  std::ifstream in = std::ifstream(fileName, std::ios::binary);
  if (!in.is_open()) {
    return {};
  }
  auto pbin = google::protobuf::io::IstreamInputStream(&in);
  auto pMessage = std::make_unique<padt::brep::proto::BRepEntity>();
  BRep brep;
  bool success = true;
  std::vector<padt::brep::proto::BRepEntity> entities;
  while (padt::pbio::readDelimitedFrom(&pbin, pMessage.get())) {
    entities.push_back(*pMessage);
    pMessage = std::make_unique<padt::brep::proto::BRepEntity>();
  }
  // Make sure we read the whole file
  if (!in.eof()) {
    return false;
  }
  // Delegate to the entity stream builder
  buildBRepFromEntityStream(entities);
  return success;
}

bool BRep::buildBRepFromEntityStream(
    const std::vector<padt::brep::proto::BRepEntity> &Entities) {
  reset();
  resetGeneratedHandleId(Entities);
  bool success = true;
  std::for_each(
      begin(Entities), end(Entities),
      [&success, this](const auto &e) { success &= addBRepEntity(e); });
  success &= buildBottomUpTopology();
  return success;
}

void BRep::resetGeneratedHandleId(
    const std::vector<padt::brep::proto::BRepEntity> &entities) {
  std::ptrdiff_t id = -1;
  for (auto entity : entities) {
    if (entity.has_assembly()) {
      id = std::max(id, entity.assembly().id());
    } else if (entity.has_part()) {
      id = std::max(id, entity.part().id());
    } else if (entity.has_body()) {
      id = std::max(id, entity.body().id());
    } else if (entity.has_face()) {
      id = std::max(id, entity.face().id());
    } else if (entity.has_edge()) {
      id = std::max(id, entity.edge().id());
    } else if (entity.has_vertex()) {
      id = std::max(id, entity.vertex().id());
    } else {
    }
  }
  GeneratedHandleId_ = static_cast<int>(id + 1);
}

bool BRep::addBRepEntity(const padt::brep::proto::BRepEntity &entity) {
  if (entity.has_assembly()) {
    return addAssembly(entity.assembly());
  } else if (entity.has_part()) {
    return addPart(entity.part());
  } else if (entity.has_body()) {
    return addBody(entity.body());
  } else if (entity.has_face()) {
    return addFace(entity.face());
  } else if (entity.has_edge()) {
    return addEdge(entity.edge());
  } else if (entity.has_vertex()) {
    return addVertex(entity.vertex());
  } else {
    std::cerr << "Unknown message type" << std::endl;
    return false;
  }
}
bool BRep::addAssembly(const padt::brep::proto::Assembly &assembly) {
  AssemblyData data;
  std::transform(assembly.parts().begin(), assembly.parts().end(),
                 std::back_inserter(data.Parts_),
                 [](const auto &pId) -> PartHandle { return PartHandle(pId); });
  Assemblies_.insert_or_assign(AssemblyHandle(assembly.id()), data);
  return true;
}

bool BRep::addPart(const padt::brep::proto::Part &part) {
  PartData data;
  std::transform(part.bodies().begin(), part.bodies().end(),
                 std::back_inserter(data.Bodies_),
                 [](const auto &bId) -> BodyHandle { return BodyHandle(bId); });
  Parts_.insert_or_assign(PartHandle(part.id()), data);
  return true;
}

bool BRep::addBody(const padt::brep::proto::Body &body) {
  BodyData data;
  std::transform(body.faces().begin(), body.faces().end(),
                 std::back_inserter(data.Faces_),
                 [](const auto &fId) -> FaceHandle { return FaceHandle(fId); });
  // Create the shells and shell store
  for (const auto &s : body.shells()) {
    ShellData shellData;
    std::transform(
        s.faces().begin(), s.faces().end(),
        std::back_inserter(shellData.Faces_),
        [](const auto &fId) -> FaceHandle { return FaceHandle(fId); });
  }
  Bodies_.insert_or_assign(BodyHandle(body.id()), data);
  return true;
}

bool BRep::addFace(const padt::brep::proto::Face &face) {
  FaceData data;
  // Add in the points
  data.PointStart_ = V_.rows();
  data.PointSize_ = face.surface().points().size();
  V_.conservativeResize(V_.rows() + data.PointSize_, 3);
  int row = data.PointStart_;
  for (auto v : face.surface().points()) {
    V_(row, 0) = v.x();
    V_(row, 1) = v.y();
    V_(row, 2) = v.z();
    row++;
  }

  // Add in the facets
  data.FacetStart_ = F_.rows();
  data.FacetSize_ = face.surface().triangles().size();
  F_.conservativeResize(F_.rows() + data.FacetSize_, 3);
  row = data.FacetStart_;
  for (auto t : face.surface().triangles()) {
    F_(row, 0) = t.i();
    F_(row, 1) = t.j();
    F_(row, 2) = t.k();
    row++;
  }
  // Add in the parameterization
  data.ParamStart_ = FaceParams_.rows();
  data.ParamSize_ = face.surface().parameters().size();
  FaceParams_.conservativeResize(FaceParams_.rows() + data.ParamSize_, 2);
  row = data.ParamStart_;
  for (auto p : face.surface().parameters()) {
    FaceParams_(row, 0) = p.u();
    FaceParams_(row, 1) = p.v();
    row++;
  }
  // Add in the edges
  std::transform(face.edges().begin(), face.edges().end(),
                 std::back_inserter(data.Edges_),
                 [](auto eId) -> EdgeHandle { return EdgeHandle(eId); });

  // Create the loops and add them to the loop store.
  for (const auto &l : face.loops()) {
    LoopData loopData;
    std::transform(l.edges().begin(), l.edges().end(),
                   std::back_inserter(loopData.Edges_),
                   [](auto eId) -> EdgeHandle { return EdgeHandle(eId); });
    LoopHandle lh = generateNewLoopHandle();
    data.Loops_.push_back(lh);
    Loops_.insert_or_assign(lh, loopData);
  }
  Faces_.insert_or_assign(FaceHandle(face.id()), data);
  return true;
}

bool BRep::addEdge(const padt::brep::proto::Edge &edge) {
  EdgeData data;
  data.PointStart_ = V_.rows();
  data.PointSize_ = edge.curve().points().size();
  V_.conservativeResize(V_.rows() + data.PointSize_, 3);
  int row = data.PointStart_;
  for (auto v : edge.curve().points()) {
    V_(row, 0) = v.x();
    V_(row, 1) = v.y();
    V_(row, 2) = v.z();
    row++;
  }

  // Add in the parameterization
  data.ParamStart_ = EdgeParams_.rows();
  data.ParamSize_ = edge.curve().parameters().size();
  EdgeParams_.conservativeResize(EdgeParams_.rows() + data.ParamSize_, 1);
  row = data.ParamStart_;
  for (auto p : edge.curve().parameters()) {
    EdgeParams_(row) = p;
    row++;
  }

  data.startVertex_ =
      edge.start() >= 0 ? VertexHandle(edge.start()) : VertexHandle();
  data.endVertex_ = edge.end() >= 0 ? VertexHandle(edge.end()) : VertexHandle();

  Edges_.insert_or_assign(EdgeHandle(edge.id()), data);
  return true;
}

bool BRep::addVertex(const padt::brep::proto::Vertex &vertex) {
  VertexData data;
  data.PointIndex_ = V_.rows();
  V_.conservativeResize(V_.rows() + 1, 3);
  V_(data.PointIndex_, 0) = vertex.point().x();
  V_(data.PointIndex_, 1) = vertex.point().y();
  V_(data.PointIndex_, 2) = vertex.point().z();
  Vertices_.insert_or_assign(VertexHandle(vertex.id()), data);
  return true;
}

bool BRep::buildBottomUpTopology() {
  std::unordered_map<VertexHandle, std::set<EdgeHandle>,
                     HandleHash<VertexHandle>>
      VertexToEdges;
  std::unordered_map<EdgeHandle, std::set<FaceHandle>, HandleHash<EdgeHandle>>
      EdgesToFaces;
  std::unordered_map<FaceHandle, std::set<BodyHandle>, HandleHash<FaceHandle>>
      FacesToBodies;
  // Topology for mapping a vertex to the connected edges
  for (const auto &[eH, eData] : Edges_) {
    if (eData.endVertex_.isValid()) {
      VertexToEdges[eData.endVertex_].insert(eH);
    }
    if (eData.startVertex_.isValid()) {
      VertexToEdges[eData.startVertex_].insert(eH);
    }
  }
  for (auto &[vH, vData] : Vertices_) {
    std::copy(VertexToEdges[vH].begin(), VertexToEdges[vH].end(),
              std::back_inserter(vData.Edges_));
  }
  // Topology for mapping an edge to the connected faces
  for (const auto &[fH, fData] : Faces_) {
    for (const auto &eH : fData.Edges_) {
      EdgesToFaces[eH].insert(fH);
    }
  }
  for (auto &[eH, eData] : Edges_) {
    std::copy(EdgesToFaces[eH].begin(), EdgesToFaces[eH].end(),
              std::back_inserter(eData.Faces_));
  }
  // Topology for mapping a face to the connected bodies
  for (const auto &[bH, bData] : Bodies_) {
    for (const auto &fH : bData.Faces_) {
      FacesToBodies[fH].insert(bH);
    }
  }
  for (auto &[fH, fData] : Faces_) {
    std::copy(FacesToBodies[fH].begin(), FacesToBodies[fH].end(),
              std::back_inserter(fData.Bodies_));
  }

  // Build up the edges and vertices list for a given body
  for (auto &[bH, bData] : Bodies_) {
    std::set<EdgeHandle> bodyEdges;
    for (const auto &fH : bData.Faces_) {
      const auto &faceData = Faces_.at(fH);
      std::copy(faceData.Edges_.begin(), faceData.Edges_.end(),
                std::inserter(bodyEdges, bodyEdges.end()));
    }
    std::copy(bodyEdges.begin(), bodyEdges.end(),
              std::back_inserter(bData.Edges_));
    std::set<VertexHandle> bodyVertices;

    for (const auto eH : bodyEdges) {
      const auto &edgeData = Edges_.at(eH);
      if (edgeData.startVertex_.isValid()) {
        bodyVertices.insert(edgeData.startVertex_);
      }
      if (edgeData.endVertex_.isValid()) {
        bodyVertices.insert(edgeData.endVertex_);
      }
    }
    std::copy(bodyVertices.begin(), bodyVertices.end(),
              std::back_inserter(bData.Vertices_));
  }

  return true;
}

Eigen::AlignedBox3d BRep::boundingBox(const AssemblyHandle &h) const {
  if (AssemblyBBoxCache_.count(h) > 0) {
    return AssemblyBBoxCache_.at(h);
  }
  if (Assemblies_.count(h) == 0) {
    return Eigen::AlignedBox3d();
  }
  const auto &assemblyData = Assemblies_.at(h);
  std::vector<Eigen::AlignedBox3d> boundingBoxes;
  std::transform(assemblyData.Parts_.begin(), assemblyData.Parts_.end(),
                 std::back_inserter(boundingBoxes),
                 [this](const auto &pId) -> Eigen::AlignedBox3d {
                   return boundingBox(pId);
                 });

  Eigen::AlignedBox3d bbox = std::accumulate(
      boundingBoxes.begin(), boundingBoxes.end(), Eigen::AlignedBox3d(),
      [](const auto &b1, const auto &b2) -> Eigen::AlignedBox3d {
        Eigen::AlignedBox3d b = b1;
        b.extend(b2);
        return b;
      });
  AssemblyBBoxCache_.insert_or_assign(h, bbox);
  return bbox;
}

Eigen::AlignedBox3d BRep::boundingBox(const PartHandle &h) const {
  if (PartBBoxCache_.count(h) > 0) {
    return PartBBoxCache_.at(h);
  }
  if (Parts_.count(h) == 0) {
    return Eigen::AlignedBox3d();
  }
  const auto &partData = Parts_.at(h);
  std::vector<Eigen::AlignedBox3d> boundingBoxes;
  std::transform(partData.Bodies_.begin(), partData.Bodies_.end(),
                 std::back_inserter(boundingBoxes),
                 [this](const auto &bId) -> Eigen::AlignedBox3d {
                   return boundingBox(bId);
                 });

  Eigen::AlignedBox3d bbox = std::accumulate(
      boundingBoxes.begin(), boundingBoxes.end(), Eigen::AlignedBox3d(),
      [](const auto &b1, const auto &b2) -> Eigen::AlignedBox3d {
        Eigen::AlignedBox3d b = b1;
        b.extend(b2);
        return b;
      });
  PartBBoxCache_.insert_or_assign(h, bbox);
  return bbox;
}

Eigen::AlignedBox3d BRep::boundingBox(const BodyHandle &h) const {
  if (BodyBBoxCache_.count(h) > 0) {
    return BodyBBoxCache_.at(h);
  }
  if (Bodies_.count(h) == 0) {
    return Eigen::AlignedBox3d();
  }
  const auto &bodyData = Bodies_.at(h);
  std::vector<Eigen::AlignedBox3d> boundingBoxes;
  std::transform(bodyData.Faces_.begin(), bodyData.Faces_.end(),
                 std::back_inserter(boundingBoxes),
                 [this](const auto &fId) -> Eigen::AlignedBox3d {
                   return boundingBox(fId);
                 });

  Eigen::AlignedBox3d bbox = std::accumulate(
      boundingBoxes.begin(), boundingBoxes.end(), Eigen::AlignedBox3d(),
      [](const auto &b1, const auto &b2) -> Eigen::AlignedBox3d {
        Eigen::AlignedBox3d b = b1;
        b.extend(b2);
        return b;
      });
  BodyBBoxCache_.insert_or_assign(h, bbox);
  return bbox;
}

Eigen::AlignedBox3d BRep::boundingBox(const FaceHandle &h) const {
  if (FaceBBoxCache_.count(h) > 0) {
    return FaceBBoxCache_.at(h);
  }
  if (Faces_.count(h) == 0) {
    return Eigen::AlignedBox3d();
  }
  auto [fV, fI] = faceGeometry(h);
  Eigen::Vector3d min = fV.colwise().minCoeff();
  Eigen::Vector3d max = fV.colwise().maxCoeff();
  Eigen::AlignedBox3d bbox(min, max);
  FaceBBoxCache_.insert_or_assign(h, bbox);
  return bbox;
}

Eigen::AlignedBox3d BRep::boundingBox(const EdgeHandle &h) const {
  if (EdgeBBoxCache_.count(h) > 0) {
    return EdgeBBoxCache_.at(h);
  }
  if (Edges_.count(h) == 0) {
    return Eigen::AlignedBox3d();
  }
  auto eV = edgeGeometry(h);
  Eigen::Vector3d min = eV.colwise().minCoeff();
  Eigen::Vector3d max = eV.colwise().maxCoeff();
  Eigen::AlignedBox3d bbox(min, max);
  EdgeBBoxCache_.insert_or_assign(h, bbox);
  return bbox;
}
}  // namespace padt::brep