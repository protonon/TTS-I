#ifndef DelaunayDriver_h
#define DelaunayDriver_h

#include "DelaunayVertex.hpp"
#include "DelaunayTriangle.hpp"
#include "FakeVertices.hpp"
#include "PointManager.hpp"
#include <string>

using std::string;

class DelaunayDriver
{
public:
    DelaunayDriver();
    virtual ~DelaunayDriver();

    void init(const PointManager &);

    void reinit();

    void run();

    void calcCircumcenter();

    void output(const string &);
    
    void getCircumcenter(double [], double []);

    List<DelaunayVertex> DVT;
    List<DelaunayTriangle> DT;

protected:
    void getThreeRandomIndices(int []);

    /*
     * Function:
     *   initDelaunayTriangle
     * Purpose:
     *   Initialize the first eight Delaunay triangles by randomly selecting
     *   three points, and add three fake points that are antipodal to the
     *   three real ones.
     * Return value:
     *   No return value
     */
    void initDelaunayTriangle(int []);

    /*
     * Function:
     *   initPIT
     * Purpose:
     *   Initialize the "point-in-triangle".
     * Return value:
     *   No return value
     */
    void initPIT();

    /*
     * Function:
     *   updatePIT
     * Purpose:
     *   Update the "point-in-triangle" after insert one point.
     * Return value:
     *   No return value
     */
    void updatePIT();

    /*
     * Function:
     *   insertRestPoints
     * Purpose:
     *   Insert the rest points to the first three points.
     * Return value:
     *   No return value
     */
    void insertRestPoints();

    /*
     * Function:
     *   insertPoint
     * Purpose:
     *   Insert one point.
     * Return value:
     *   No return value
     */
    void insertPoint(DelaunayVertex *);

    /*
     * Function:
     *   validate
     * Purpose:
     *   Validate a triangle according to the empty-circumcircle rule.
     * Return value:
     *   No return value
     */
    void validate(DelaunayTriangle *);
    
    /*
     * Function:
     *   flip**
     * Purpose:
     *   Modify the Delaunay triangles by replacing the old triangles with
     *   new ones. New triangles will be added in the triangle list.
     * Return value:
     *   No return value
     */
    void flip13(DelaunayTriangle *, DelaunayVertex *);
    void flip24(DelaunayTriangle *, DelaunayTriangle *, DelaunayVertex *);
    void flip22(DelaunayTriangle *, DelaunayTriangle *,
                DelaunayTriangle **,DelaunayTriangle **, int []);
    void flip31(DelaunayTriangle *, DelaunayTriangle *, DelaunayTriangle *,
                DelaunayTriangle **, int []);
    
    /*
     * Function:
     *   deleteDVT
     * Purpose:
     *   Delete a Delaunay vertex.
     * Return value:
     *   No return value
     */
    void deleteDVT(DelaunayVertex *);
    /*
     * Function:
     *   deleteFake
     * Purpose:
     *   After the Delaunay triangulation, delete the fake vertices and
     *   corresponding triangles.
     * Return value:
     *   No return value
     */
    void deleteFake();
    
    void deleteObsoleteDT();
    void deleteTemporalDT();
    void extractTopology();

    FakeVertices fake;
    List<DelaunayTrianglePointer> obsoleteDT;
    List<DelaunayTrianglePointer> temporalDT;
};

#endif
