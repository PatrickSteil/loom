// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosip@informatik.uni-freiburg.de>

#include <proj_api.h>
#include <string>
#include <set>
#include "transitgraph.h"
#include "edge.h"

using namespace transitmapper;
using namespace graph;


// _____________________________________________________________________________
TransitGraph::TransitGraph(const std::string& name, const std::string& proj)
: _name(name) {
  _bbox = boost::geometry::make_inverse<boost::geometry::model::box<util::geo::Point> >();
  _proj = pj_init_plus(proj.c_str());
}

// _____________________________________________________________________________
TransitGraph::~TransitGraph() {
  // an edge is _deleted_ if either the from or to node is deleted!
  for (auto n : _nodes) {
    //delete n;
  }

  // clean up projection stuff
  pj_free(_proj);
}

// _____________________________________________________________________________
Node* TransitGraph::addNode(Node* n) {
  // expand the bounding box to hold this new node
  boost::geometry::expand(_bbox, n->getPos());

  return *(_nodes.insert(n)).first;
}

// _____________________________________________________________________________
Edge* TransitGraph::addEdge(Node* from, Node* to) {
  Edge* e = new Edge(from, to);
  from->addEdge(e);
  to->addEdge(e);
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
    s->getParentStation()
  );

  return getNodeByStop(s);
}

// _____________________________________________________________________________
Node* TransitGraph::getNodeByStop(const gtfs::Stop* s) const {
  for (const auto n : _nodes) {
    if (n->getStop() == s) {
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
void TransitGraph::deleteNode(Node* n) {
  _nodes.erase(n);
}

// _____________________________________________________________________________
projPJ TransitGraph::getProjection() const {
  return _proj;
}

// _____________________________________________________________________________
const boost::geometry::model::box<util::geo::Point>& TransitGraph::getBoundingBox() const {
  return _bbox;
}
