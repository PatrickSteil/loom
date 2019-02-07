// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef GTFS2GEO_GRAPH_EDGEPL_H_
#define GTFS2GEO_GRAPH_EDGEPL_H_

#include <vector>
#include "gtfs2geo/graph/EdgeTripGeom.h"
#include "util/geo/PolyLine.h"
#include "util/Nullable.h"
#include "ad/cppgtfs/gtfs/Trip.h"
#include "util/geo/GeoGraph.h"
#include "util/json/Writer.h"
#include "gtfs2geo/graph/BuildGraph.h"

using namespace ad::cppgtfs;
using util::Nullable;

namespace gtfs2geo {
namespace graph {

class EdgePL : public util::geograph::GeoEdgePL<double> {
 public:
  EdgePL(const Edge* e);
  EdgePL();

  bool addTrip(gtfs::Trip* t, Node* toNode);
  bool addTrip(gtfs::Trip* t, PolyLine<double> pl, Node* toNode);

  void setEdge(const Edge* e);

  const std::vector<EdgeTripGeom>& getEdgeTripGeoms() const;
  std::vector<EdgeTripGeom>* getEdgeTripGeoms();

  const EdgeTripGeom* getRefETG() const;
  EdgeTripGeom* getRefETG();

  void addEdgeTripGeom(const EdgeTripGeom& e);

  void simplify();
  void setTo(Node* to);

  const util::geo::Line<double>* getGeom() const;
  util::json::Dict getAttrs() const;
 private:
  // Map of EdgeTripGeometries in this graph edge.
  // An EdgeTripGeometry is a geometry holding N trips.
  // This is meant as a multi-stage structure, where in the first
  // (trivial) stage, each EdgeTripGeometry holds exactly 1 trip.
  //
  // In a 2nd step, the EdgeTripGeometries are combined based on
  // geometrical equivalence.
  //
  // In a 3rd step, we split those edges with |_tripsContained| > 1
  // into single edges. This creates either multiple distinct edges from
  // _from to _to, OR, in case the geometries have partial equivalence,
  // introduces new topological nodes which mark the position where two
  // lines part or join.
  std::vector<EdgeTripGeom> _tripsContained;

  void combineIncludedGeoms();
  void averageCombineGeom();

  const Edge* _e;
};

}}

#endif  // GTFS2GEO_GRAPH_EDGEPL_H_

