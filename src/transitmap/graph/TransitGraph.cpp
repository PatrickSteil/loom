// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#include <proj_api.h>
#include <string>
#include <set>
#include "./TransitGraph.h"
#include "./Edge.h"
#include "./OrderingConfiguration.h"

using transitmapper::graph::TransitGraph;
using transitmapper::graph::Node;
using transitmapper::graph::Edge;
using transitmapper::util::geo::Point;
using transitmapper::graph::Configuration;
using bgeo::make_inverse;

// _____________________________________________________________________________
TransitGraph::TransitGraph(const std::string& name, const std::string& proj)
: _name(name) {
  _bbox = make_inverse<bgeo::model::box<Point> >();
  _proj = pj_init_plus(proj.c_str());
}

// _____________________________________________________________________________
TransitGraph::~TransitGraph() {
  // an edge is _deleted_ if either the from or to node is deleted!
  // thus, we don't have to delete edges separately here
  for (auto n : _nodes) {
    delete n;
  }

  // clean up projection stuff
  pj_free(_proj);
}

// _____________________________________________________________________________
const Configuration& TransitGraph::getConfig() const {
  return _config;
}

// _____________________________________________________________________________
void TransitGraph::setConfig(const Configuration& c) {
  _config = c;
}

// _____________________________________________________________________________
double TransitGraph::getScore() const {
  double ret = 0;

  for (auto n : getNodes()) {
    ret += n->getScore(_config);
  }

  return ret;
}

// _____________________________________________________________________________
double TransitGraph::getScore(const Configuration& c) const {
  double ret = 0;

  for (auto n : getNodes()) {
    ret += n->getScore(c);
  }

  return ret;
}

// _____________________________________________________________________________
Node* TransitGraph::addNode(Node* n) {
  auto ins = _nodes.insert(n);
  if (ins.second) {
    // expand the bounding box to hold this new node
    bgeo::expand(_bbox, n->getPos());
  }
  return *ins.first;
}

// _____________________________________________________________________________
Edge* TransitGraph::addEdge(Node* from, Node* to, geo::PolyLine pl, double w,
    double s) {
  if (from == to) return 0;
  Edge* e = getEdge(from, to);
  if (!e) {
    e = new Edge(from, to, pl, w, s);
    from->addEdge(e);
    to->addEdge(e);
  }
  return e;
}

// _____________________________________________________________________________
Edge* TransitGraph::getEdge(Node* from, Node* to) {
  for (auto e : from->getAdjListOut()) {
    if (e->getTo() == to) return e;
  }

  // also search in the opposite direction, we are handling an undirected
  // graph here
  for (auto e : from->getAdjListIn()) {
    if (e->getFrom() == to) return e;
  }

  return 0;
}

// _____________________________________________________________________________
const std::set<Node*>& TransitGraph::getNodes() const {
  return _nodes;
}

// _____________________________________________________________________________
std::set<Node*>* TransitGraph::getNodes() {
  return &_nodes;
}

// _____________________________________________________________________________
Node* TransitGraph::getNodeByStop(const gtfs::Stop* s, bool getParent) const {
  if (getParent && s->getParentStation()) return getNodeByStop(
    s->getParentStation());

  return getNodeByStop(s);
}

// _____________________________________________________________________________
Node* TransitGraph::getNodeByStop(const gtfs::Stop* s) const {
  for (const auto n : _nodes) {
    if (n->getStops().find(const_cast<gtfs::Stop*>(s)) != n->getStops().end()) {
      return n;
    }
  }
  return 0;
}

// _____________________________________________________________________________
bool TransitGraph::containsNode(Node* n) const {
  return  _nodes.find(n) != _nodes.end();
}

// _____________________________________________________________________________
void TransitGraph::deleteEdge(Node* from, Node* to) {
  Edge* toDel = getEdge(from, to);
  if (!toDel) return;

  from->removeEdge(toDel);
  to->removeEdge(toDel);

  assert(!getEdge(from, to));

  delete toDel;
}

// _____________________________________________________________________________
void TransitGraph::deleteNode(Node* n) {
  _nodes.erase(n);
}

// _____________________________________________________________________________
projPJ TransitGraph::getProjection() const {
  return _proj;
}

// _____________________________________________________________________________
const bgeo::model::box<Point>& TransitGraph::getBoundingBox() const {
  return _bbox;
}

// _____________________________________________________________________________
Node* TransitGraph::getNearestNode(const Point& p, double maxD) const {
  double curD = DBL_MAX;;
  Node* curN = 0;
  for (auto n : _nodes) {
    double d = bgeo::distance(n->getPos(), p);
    if (d < maxD && d < curD) {
      curN = n;
      curD = d;
    }
  }

  return curN;
}
