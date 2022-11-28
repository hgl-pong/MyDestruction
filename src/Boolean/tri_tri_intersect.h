/*
*
*  Triangle-Triangle Overlap Test Routines
*  July, 2002
*  Updated December 2003
*
*  This file contains C implementation of algorithms for
*  performing two and three-dimensional triangle-triangle intersection test
*  The algorithms and underlying theory are described in
*
* "Fast and Robust Triangle-Triangle Overlap Test
*  Using Orientation Predicates"  P. Guigue - O. Devillers
*
*  Journal of Graphics Tools, 8(1), 2003
*
*  Several geometric predicates are defined.  Their parameters are all
*  points.  Each point is an array of two or three float precision
*  floating point numbers. The geometric predicates implemented in
*  this file are:
*
*    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)
*    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)
*
*    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,
*                                     coplanar,source,target)
*
*       is a version that computes the segment of intersection when
*       the triangles overlap (and are not coplanar)
*
*    each function returns 1 if the triangles (including their
*    boundary) intersect, otherwise 0
*
*
*  Other information are available from the Web page
*  http://www.acm.org/jgt/papers/GuigueDevillers03/
*
*/

#ifndef TRI_TRI_INTERSECT_H
#define TRI_TRI_INTERSECT_H

#ifdef __cplusplus
extern "C" {
#endif

int tri_tri_intersection_test_3d(float p1[3], float q1[3], float r1[3],
								 float p2[3], float q2[3], float r2[3],
								 int * coplanar,
								 float source[3],float target[3]);

#ifdef __cplusplus
}
#endif

#endif
