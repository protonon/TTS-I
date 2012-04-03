#ifndef Vertex_h
#define Vertex_h

#include "Point.h"
#include "List.h"
class PolygonManager;
#ifdef TTS_ONLINE
class MeshManager;
class FlowManager;
#include "ApproachDetector.h"
#include "VertexTags.h"
#endif

class Edge;
class EdgePointer;

class Vertex : public Point, public ListElement<Vertex>
{
public:
	Vertex();
	virtual ~Vertex();

    void reinit();
    void clean();

    void linkEdge(Edge *);
    void unlinkEdge(Edge *);
    void handoverEdges(Vertex *, PolygonManager &);
#ifdef TTS_ONLINE
    void handoverEdges(Vertex *, MeshManager &, const FlowManager &,
                       PolygonManager &);
    void setHostEdge(Edge *edge) { hostEdge = edge; }
    Edge *getHostEdge() const { return hostEdge; }
#endif

    bool isJoint() const { return linkedEdges.size() > 2; }

    Vertex &operator=(const Vertex &);

    void dump(int indentLevel = 0);

#ifdef TTS_ONLINE
    ApproachDetector::VertexAgent detectAgent;
    VertexTags tags;
    Edge *hostEdge; // for test point
#endif
	List<EdgePointer> linkedEdges;
};

#endif
