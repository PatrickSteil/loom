// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef TRANSITMAP_GRAPH_OPTIM_OPTGRAPH_H_
#define TRANSITMAP_GRAPH_OPTIM_OPTGRAPH_H_

#include <set>
#include <string>

#include "transitmap/graph/Edge.h"
#include "transitmap/graph/TransitGraph.h"
#include "util/graph/UndirGraph.h"
#include "util/json/Writer.h"

using transitmapper::graph::TransitGraph;
using transitmapper::graph::Node;
using transitmapper::graph::Edge;
using util::graph::UndirGraph;

namespace transitmapper {
namespace optim {

struct OptNodePL;
struct OptEdgePL;

typedef util::graph::Node<OptNodePL, OptEdgePL> OptNode;
typedef util::graph::Edge<OptNodePL, OptEdgePL> OptEdge;

struct EtgPart {
  Edge* etg;
  bool dir;
  size_t order;

  // there is another edge with determines the
  // ordering in this edge - important to prevent double
  // writing of ordering later on
  bool wasCut;

  EtgPart(Edge* etg, bool dir) : etg(etg), dir(dir), order(0), wasCut(false){};
  EtgPart(Edge* etg, bool dir, size_t order, bool wasCut)
      : etg(etg), dir(dir), order(order), wasCut(wasCut){};
};

struct OptEdgePL {
  OptEdgePL() : depth(0){};

  // all original ETGs from the transit graph contained in this edge
  // Guarantee: they are all equal in terms of (directed) routes
  std::vector<EtgPart> etgs;

  size_t getCardinality() const;
  std::string toStr() const;
  const std::vector<graph::RouteOccurance>& getRoutes() const;

  size_t depth;

  // partial routes
  // For the ETGs contained in .etgs, only these route occurances are
  // actually contained in this edge. Their relative ordering is defined by
  // .order
  std::vector<graph::RouteOccurance> partialRoutes;

  std::string getStrRepr() const;

  const util::geo::Line<double>* getGeom();
  util::json::Dict getAttrs();
};

struct OptNodePL {
  const Node* node;
  util::geo::Point<double> p;

  // the edges arriving at this node, in clockwise fashion, based
  // on the geometry in the original graph
  std::vector<OptEdge*> orderedEdges;

  OptNodePL(util::geo::Point<double> p) : node(0), p(p){};
  OptNodePL(const Node* node) : node(node), p(node->getPos()){};
  OptNodePL() : node(0){};

  const util::geo::Point<double>* getGeom();
  util::json::Dict getAttrs();
};

class OptGraph : public UndirGraph<OptNodePL, OptEdgePL> {
 public:
  explicit OptGraph(TransitGraph* toOptim);

  TransitGraph* getGraph() const;

  size_t getNumNodes() const;
  size_t getNumNodes(bool topo) const;
  size_t getNumEdges() const;
  size_t getNumRoutes() const;
  size_t getMaxCardinality() const;

  double getMaxCrossPen() const;
  double getMaxSplitPen() const;

  void simplify();
  void untangle();

  static graph::Edge* getAdjEdg(const OptEdge* e, const OptNode* n);
  static EtgPart getAdjEtgp(const OptEdge* e, const OptNode* n);
  static std::vector<graph::RouteOccurance> getCtdRoutesIn(
      const graph::Route* r, const Node* dir, const OptEdge* fromEdge,
      const OptEdge* toEdge);

  // apply splitting rules
  void split();

 private:
  TransitGraph* _g;

  OptNode* getNodeForTransitNode(const Node* tn) const;

  void build();
  void writeEdgeOrder();
  void updateEdgeOrder(OptNode* n);
  bool simplifyStep();

  bool untangleYStep();
  bool untanglePartialYStep();
  bool untangleDogBoneStep();

  bool isYAt(OptEdge* e, OptNode* n) const;
  bool isPartialYAt(OptEdge* e, OptNode* n) const;

  bool isDogBone(OptEdge* e) const;

  static EtgPart getFirstEdg(const OptEdge*);
  static EtgPart getLastEdg(const OptEdge*);

  static OptEdgePL getView(OptEdge* parent, OptNode* origin, OptEdge* leg,
                           size_t offset);
  static OptEdgePL getPartialView(OptEdge* parent, OptNode* origin,
                                  OptEdge* leg, size_t offset);

  static bool dirRouteContains(const OptEdge* a, const OptEdge* b);
  static bool dirRouteEqualIn(const OptEdge* a, const OptEdge* b);
  static bool dirContinuedOver(const OptEdge* a, const OptEdge* b,
                               const OptEdge* c);
  static bool dirPartialContinuedOver(const OptEdge* a, const OptEdge* b);
  static bool dirContinuedOver(const graph::RouteOccurance& ro,
                               const OptEdge* a, const OptEdge* b);

  static std::vector<graph::RouteOccurance> getCtdRoutesIn(
      const OptEdge* fromEdge, const OptEdge* toEdge);

  static OptNode* sharedNode(const OptEdge* a, const OptEdge* b);

  static std::vector<OptEdge*> clockwEdges(OptEdge* noon, OptNode* n);
  static std::vector<OptEdge*> partialClockwEdges(OptEdge* noon, OptNode* n);
};

// compare the orientation of two edges adjacent to some shared node
inline bool cmpEdge(const OptEdge* a, const OptEdge* b) {
  double angA, angB;

  // n is the shared node
  OptNode* n = 0;
  if (a->getFrom() == b->getFrom() || a->getFrom() == b->getTo())
    n = a->getFrom();
  else
    n = a->getTo();
  assert(n->pl().node);

  auto tgEdgeA = OptGraph::getAdjEdg(a, n);
  assert(tgEdgeA);
  angA = n->pl().node->getNodeFrontFor(tgEdgeA)->getOutAngle();

  auto tgEdgeB = OptGraph::getAdjEdg(b, n);
  assert(tgEdgeB);
  angB = n->pl().node->getNodeFrontFor(tgEdgeB)->getOutAngle();

  if (tgEdgeA == tgEdgeB) {
    // if these edges originally came from the same node front, use their
    // internal ordering
    // TODO: what if the edge is reversed?
    if ((a->getFrom() == n && b->getFrom() == n)) {
      if (OptGraph::getAdjEtgp(a, n).dir) {
        assert(OptGraph::getAdjEtgp(b, n).dir);
        return OptGraph::getAdjEtgp(a, n).order <
               OptGraph::getAdjEtgp(b, n).order;
      } else {
        return OptGraph::getAdjEtgp(a, n).order >
               OptGraph::getAdjEtgp(b, n).order;
      }
    } else if ((a->getTo() == n && b->getTo() == n)) {
      if (OptGraph::getAdjEtgp(a, n).dir) {
        assert(OptGraph::getAdjEtgp(b, n).dir);
        return OptGraph::getAdjEtgp(a, n).order >
               OptGraph::getAdjEtgp(b, n).order;
      } else {
        return OptGraph::getAdjEtgp(a, n).order <
               OptGraph::getAdjEtgp(b, n).order;
      }
    }
  }
  return fmod(angA + M_PI * 1.5, 2 * M_PI) > fmod(angB + M_PI * 1.5, 2 * M_PI);
}
}
}

#endif  // TRANSITMAP_GRAPH_OPTIM_OPTGRAPH_H_
