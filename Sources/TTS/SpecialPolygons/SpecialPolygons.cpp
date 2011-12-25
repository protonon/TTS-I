#include "SpecialPolygons.h"
#include "PolygonManager.h"
#include "MeshManager.h"
#include "FlowManager.h"
#include "ApproachDetector.h"
#include "PotentialCrossDetector.h"
#include "TTS.h"
#include "CommonTasks.h"

using namespace SpecialPolygons;
using namespace ApproachDetector;
using namespace PotentialCrossDetector;

void SpecialPolygons::handleLinePolygon(PolygonManager &polygonManager,
                                        Polygon *&polygon)
{
    EdgePointer *edgePointer1 = polygon->edgePointers.front();
    EdgePointer *edgePointer2 = polygon->edgePointers.back();
    EdgePointer *edgePointer3, *edgePointer4;
    Edge *edge1 = edgePointer1->edge;
    Edge *edge2 = edgePointer2->edge;
#ifdef DEBUG
    assert(((edge1->getEndPoint(FirstPoint) ==
             edge2->getEndPoint(FirstPoint)) &&
            (edge1->getEndPoint(SecondPoint) ==
             edge2->getEndPoint(SecondPoint))) ||
           ((edge1->getEndPoint(FirstPoint) ==
             edge2->getEndPoint(SecondPoint)) &&
            (edge1->getEndPoint(SecondPoint) ==
             edge2->getEndPoint(FirstPoint))));
#endif
    if (edgePointer1->orient == OrientLeft) {
        edgePointer3 = edge1->getEdgePointer(OrientRight);
    } else {
        edgePointer3 = edge1->getEdgePointer(OrientLeft);
    }
    if (edgePointer2->orient == OrientLeft) {
        edgePointer4 = edge2->getEdgePointer(OrientRight);
    } else {
        edgePointer4 = edge2->getEdgePointer(OrientLeft);
    }
    Polygon *polygon1 = edge1->getPolygon(edgePointer3->orient);
    Polygon *polygon2 = edge2->getPolygon(edgePointer4->orient);
    if (polygon1 == polygon2) {
        CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer1);
        CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer2);
        if (edgePointer3->next == edgePointer4) {
            CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer3);
            CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer4);
            CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer4->next); 
            polygon1->edgePointers.remove(edgePointer3);
            polygon1->edgePointers.remove(edgePointer4);
            polygonManager.edges.remove(edge1);
            polygonManager.edges.remove(edge2);
            polygonManager.vertices.remove(edgePointer4->getEndPoint(FirstPoint));
        } else if (edgePointer4->next == edgePointer3) {
            CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer3);
            CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer4); 
            CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer3->next);
            polygon1->edgePointers.remove(edgePointer3);
            polygon1->edgePointers.remove(edgePointer4);
            polygonManager.edges.remove(edge1);
            polygonManager.edges.remove(edge2);
            polygonManager.vertices.remove(edgePointer3->getEndPoint(FirstPoint));
        } else {
            Message message;
            message << "Line polygon is enclosed and there is another polygon "
            "on the other end!";
            REPORT_ERROR(message.str());
        }
    } else {
        edge1->setPolygon(edgePointer1->orient, polygon2);
        edge1->setEdgePointer(edgePointer1->orient, edgePointer4);
        edge2->detectAgent.handoverVertices(edge1);
        CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer1);
        CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer2);
        polygonManager.edges.remove(edge2);
    }
    polygonManager.polygons.remove(polygon);
}

void SpecialPolygons::handlePointPolygon(PolygonManager &polygonManager,
                                         Polygon *polygon)
{
#ifdef DEBUG
    assert(polygon->edgePointers.size() == 1);
#endif
    EdgePointer *edgePointer1 = polygon->edgePointers.front();
    EdgePointer *edgePointer2;
    Polygon *polygon2;
    if (edgePointer1->orient == OrientLeft) {
        edgePointer2 = edgePointer1->edge->getEdgePointer(OrientRight);
        polygon2 = edgePointer1->edge->getPolygon(OrientRight);
    } else {
        edgePointer2 = edgePointer1->edge->getEdgePointer(OrientLeft);
        polygon2 = edgePointer1->edge->getPolygon(OrientLeft);
    }
    polygon2->edgePointers.remove(edgePointer2);
#ifdef DEBUG
    assert(edgePointer1->edge->getEndPoint(FirstPoint) ==
           edgePointer1->edge->getEndPoint(SecondPoint));
#endif
    polygonManager.polygons.remove(polygon);
    polygonManager.edges.remove(edgePointer1->edge);
    CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer1);
    CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer2);
}

void SpecialPolygons::handleSlimPolygon(MeshManager &meshManager,
                                        const FlowManager &flowManager,
                                        PolygonManager &polygonManager,
                                        Polygon *&polygon)
{
    static const double smallAngle = 20.0/Rad2Deg;
    static const double smallDistance = 0.01/Rad2Deg*Sphere::radius;
    // Note: Currently, we only deal with triangles.
    if (polygon->edgePointers.size() == 3) {
        // 1. calculate angles if necessary
        EdgePointer *edgePointer = polygon->edgePointers.front();
        for (int i = 0; i < polygon->edgePointers.size(); ++i) {
            if (edgePointer->getAngle() == UNSET_ANGLE) {
                edgePointer->calcAngle();
                CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer);
            }
            edgePointer = edgePointer->next;
        }
        // 2. set up the edge pointers
        EdgePointer *edgePointer1 = NULL;
        EdgePointer *edgePointer2 = NULL;
        EdgePointer *edgePointer3 = NULL;
        edgePointer = polygon->edgePointers.front();
        for (int i = 0; i < polygon->edgePointers.size()+1; ++i) {
            if (edgePointer->getAngle() < smallAngle) {
                edgePointer1 = edgePointer;
                edgePointer2 = edgePointer->next;
                edgePointer3 = edgePointer->prev;
                break;
            }
            edgePointer = edgePointer->next;
        }
        if (edgePointer1 != NULL) {
            // 3. set up the help pointers
            Polygon *polygon1 = edgePointer1->getPolygon(OrientRight);
            Polygon *polygon2 = edgePointer2->getPolygon(OrientRight);
            Polygon *polygon3 = edgePointer3->getPolygon(OrientRight);
            Vertex *vertex1 = edgePointer1->getEndPoint(FirstPoint);
            Vertex *vertex2 = edgePointer2->getEndPoint(FirstPoint);
            Vertex *vertex3 = edgePointer3->getEndPoint(FirstPoint);
            Edge *edge1 = edgePointer1->edge;
            Edge *edge2 = edgePointer2->edge;
            Edge *edge3 = edgePointer3->edge;
            edgePointer1 = edgePointer1->getNeighborEdgePointer();
            edgePointer2 = edgePointer2->getNeighborEdgePointer();
            edgePointer3 = edgePointer3->getNeighborEdgePointer();
            // 4. handle the slim triangle in different scenarios
            // -----------------------------------------------------------------
            if (polygon1 != polygon3 && polygon2 != polygon3) {
                // =============================================================
                if (polygon1 == polygon2) {
#ifdef DEBUG
                    assert(edgePointer1->prev == edgePointer2);
#endif
                    if (edge3->getPolygon(OrientLeft) == polygon) {
                        edge3->setPolygon(OrientLeft, polygon1);
                        edge3->setEdgePointer(OrientLeft, edgePointer1);
                    } else {
                        edge3->setPolygon(OrientRight, polygon1);
                        edge3->setEdgePointer(OrientRight, edgePointer1);
                    }
                    edge1->detectAgent.handoverVertices(edge3);
                    edge2->detectAgent.handoverVertices(edge3);
                    polygonManager.edges.remove(edge1);
                    polygonManager.edges.remove(edge2);
                    polygonManager.vertices.remove(vertex2);
                    CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1);
                    CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1->next);
                    polygon2->edgePointers.remove(edgePointer2);
                }
                // =============================================================
                else {
                    edge1->calcLength();
                    edge2->calcLength();
                    if (edge1->getLength() < smallDistance &&
                        detectReplaceVertex(vertex2, vertex1, true) == NoCross) {
                        vertex2->handoverEdges(vertex1, meshManager,
                                               flowManager, polygonManager);
                        if (edge3->getPolygon(OrientLeft) == polygon) {
                            edge3->setPolygon(OrientLeft, polygon2);
                            edge3->setEdgePointer(OrientLeft, edgePointer2);
                        } else {
                            edge3->setPolygon(OrientRight, polygon2);
                            edge3->setEdgePointer(OrientRight, edgePointer2);
                        }
                        edge1->detectAgent.handoverVertices(edge3);
                        edge2->detectAgent.handoverVertices(edge3);
                        polygonManager.edges.remove(edge1);
                        polygonManager.edges.remove(edge2);
                        polygonManager.vertices.remove(vertex2);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer2);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer2->next);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1->next);
                        polygon1->edgePointers.remove(edgePointer1);
                    } else if (edge2->getLength() < smallDistance &&
                               detectReplaceVertex(vertex2, vertex3, true) == NoCross) {
                        vertex2->handoverEdges(vertex3, meshManager,
                                               flowManager, polygonManager);
                        if (edge3->getPolygon(OrientLeft) == polygon) {
                            edge3->setPolygon(OrientLeft, polygon1);
                            edge3->setEdgePointer(OrientLeft, edgePointer1);
                        } else {
                            edge3->setPolygon(OrientRight, polygon1);
                            edge3->setEdgePointer(OrientRight, edgePointer1);
                        }
                        edge1->detectAgent.handoverVertices(edge3);
                        edge2->detectAgent.handoverVertices(edge3);
                        polygonManager.edges.remove(edge1);
                        polygonManager.edges.remove(edge2);
                        polygonManager.vertices.remove(vertex2);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1->next);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer2->next);
                        polygon2->edgePointers.remove(edgePointer2);
                    } else {
                        polygon->dump("polygon");
                        return;
                    }
                }
                polygonManager.polygons.remove(polygon);
            }
            // -----------------------------------------------------------------
            else if (polygon1 != polygon3 && polygon2 == polygon3) {
                if (vertex3->linkedEdges.size() != 2) {
                    
                    handleEnclosedPolygons(polygonManager, polygon2,
                                           edgePointer3->getNeighborEdgePointer(),
                                           edgePointer2->getNeighborEdgePointer());
                }
                if (edge1->getPolygon(OrientLeft) == polygon) {
                    edge1->setPolygon(OrientLeft, polygon3);
                    edge1->setEdgePointer(OrientLeft, edgePointer3);
                } else {
                    edge1->setPolygon(OrientRight, polygon3);
                    edge1->setEdgePointer(OrientRight, edgePointer3);
                }
                edge2->detectAgent.handoverVertices(edge1);
                edge3->detectAgent.handoverVertices(edge1);
                polygonManager.edges.remove(edge2);
                polygonManager.edges.remove(edge3);
                polygonManager.vertices.remove(vertex3);
                CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer3);
                CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer2->next);
                CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer2);
                polygon2->edgePointers.remove(edgePointer2);
                polygonManager.polygons.remove(polygon);
            }
            // -----------------------------------------------------------------
            else if (polygon1 == polygon3 && polygon2 != polygon3) {
                if (vertex1->linkedEdges.size() == 2) {
                    if (edge2->getPolygon(OrientLeft) == polygon) {
                        edge2->setPolygon(OrientLeft, polygon3);
                        edge2->setEdgePointer(OrientLeft, edgePointer3);
                    } else {
                        edge2->setPolygon(OrientRight, polygon3);
                        edge2->setEdgePointer(OrientRight, edgePointer3);
                    }
                    edge1->detectAgent.handoverVertices(edge2);
                    edge3->detectAgent.handoverVertices(edge2);
                    polygonManager.edges.remove(edge1);
                    polygonManager.edges.remove(edge3);
                    polygonManager.vertices.remove(vertex1);
                    CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer3);
                    CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer3->next);
                    CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer1);
                    polygon1->edgePointers.remove(edgePointer1);
                } else {
                    edge2->calcLength();
                    if (edge2->getLength() < smallDistance &&
                        detectReplaceVertex(vertex2, vertex3, true) == NoCross) {
                        vertex2->handoverEdges(vertex3, meshManager,
                                               flowManager, polygonManager);
                        if (edge3->getPolygon(OrientLeft) == polygon) {
                            edge3->setPolygon(OrientLeft, polygon1);
                            edge3->setEdgePointer(OrientLeft, edgePointer1);
                        } else {
                            edge3->setPolygon(OrientRight, polygon1);
                            edge3->setEdgePointer(OrientRight, edgePointer1);
                        }
                        edge1->detectAgent.handoverVertices(edge3);
                        edge2->detectAgent.handoverVertices(edge3);
                        polygonManager.edges.remove(edge1);
                        polygonManager.edges.remove(edge2);
                        polygonManager.vertices.remove(vertex2);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer1->next);
                        CommonTasks::recordTask(CommonTasks::UpdateAngle, edgePointer2->next);
                        polygon2->edgePointers.remove(edgePointer2);
                    } else
                        return;
                }
                polygonManager.polygons.remove(polygon);
            }
            // -----------------------------------------------------------------
            else if (polygon1 == polygon3 && polygon2 == polygon3) {
                CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer1);
                CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer2);
                CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer3);
                // =============================================================
                if (vertex1->linkedEdges.size() != 2) {
#ifdef DEBUG
                    assert(vertex2->linkedEdges.size() == 2);
                    assert(vertex3->linkedEdges.size() == 2);
#endif
                    handleEnclosedPolygons(polygonManager, polygon1,
                                           edgePointer3, edgePointer1);
                }
                // =============================================================
                else if (vertex2->linkedEdges.size() != 2) {
#ifdef DEBUG
                    assert(vertex1->linkedEdges.size() == 2);
                    assert(vertex3->linkedEdges.size() == 2);
#endif
                    handleEnclosedPolygons(polygonManager, polygon1,
                                           edgePointer1, edgePointer2);
                }
                // =============================================================
                else if (vertex3->linkedEdges.size() != 2) {
#ifdef DEBUG
                    assert(vertex1->linkedEdges.size() == 2);
                    assert(vertex2->linkedEdges.size() == 2);
#endif
                    handleEnclosedPolygons(polygonManager, polygon1,
                                           edgePointer2, edgePointer3);
                }
            }
            // -----------------------------------------------------------------
            else
                REPORT_ERROR("Unhandled case!");
            polygon = NULL;
        }
    }
}

void SpecialPolygons::handleEnclosedPolygons(PolygonManager &polygonManager,
                                             Polygon *polygon1,
                                             EdgePointer *edgePointer11,
                                             EdgePointer *edgePointer12,
                                             Polygon *polygon)
{
    EdgePointer *edgePointer2;
    EdgePointer *edgePointer;
    Vertex *vertex;
    Polygon *polygon2;
    if (polygon == NULL) {
        edgePointer2 = edgePointer12->getNeighborEdgePointer();
        polygon2 = edgePointer2->getPolygon(OrientLeft);
        // ---------------------------------------------------------------------
        Polygon *polygon3;
        edgePointer = edgePointer2;
        do {
            polygon3 = edgePointer->getPolygon(OrientRight);
            if (polygon3 != polygon1 && polygon3 != NULL)
                handleEnclosedPolygons(polygonManager, polygon1,
                                       edgePointer, NULL, polygon2);
            if (edgePointer->next != edgePointer2) {
                vertex = edgePointer->getEndPoint(SecondPoint);
                polygonManager.vertices.remove(vertex);
            }
            polygonManager.edges.remove(edgePointer->edge);
            edgePointer = edgePointer->next;
        } while (edgePointer != edgePointer2);
        // ---------------------------------------------------------------------
        EdgePointer *edgePointer13 = edgePointer12->next;
        edgePointer = edgePointer11;
        while (true) {
            polygon1->edgePointers.remove(edgePointer);
            if (edgePointer == edgePointer12) break;
            edgePointer = edgePointer->next;
        }
        CommonTasks::deleteTask(CommonTasks::UpdateAngle, edgePointer13);
        edgePointer13->calcAngle();
    } else {
        edgePointer2 = edgePointer11->getNeighborEdgePointer();
        polygon2 = edgePointer2->getPolygon(OrientLeft);
        edgePointer = edgePointer2->next;
        do {
            if (edgePointer->next->getPolygon(OrientRight) != polygon) {
                vertex = edgePointer->getEndPoint(SecondPoint);
                polygonManager.vertices.remove(vertex);
            }
            if (edgePointer->getPolygon(OrientRight) == polygon) {
                EdgePointer *edgePointer3 = edgePointer->getNeighborEdgePointer();
                if (edgePointer3->orient == OrientLeft)
                    edgePointer3->edge->setPolygon(OrientRight, NULL);
                else
                    edgePointer3->edge->setPolygon(OrientLeft, NULL);
            } else if (edgePointer->getPolygon(OrientRight) == polygon1) {
                polygonManager.edges.remove(edgePointer->edge);
            } else if (edgePointer->getPolygon(OrientRight) != NULL) {
                handleEnclosedPolygons(polygonManager, polygon1,
                                       edgePointer, NULL, polygon2);
                polygonManager.edges.remove(edgePointer->edge);
            } else {
                polygonManager.edges.remove(edgePointer->edge);
            }
            edgePointer = edgePointer->next;
        } while (edgePointer != edgePointer2);
    }
    polygonManager.polygons.remove(polygon2);
}