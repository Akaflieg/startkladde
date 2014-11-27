#ifndef RECTANGLELAYOUT_H_
#define RECTANGLELAYOUT_H_

#include <QList>

/**
 * Calculates the positions of rectangles that should not overlap
 *
 * For each rectangle i, a height h_i and an optimal position yo_i is specified
 * by the user. This class calculates the positions for the rectangles in such a
 * way that no rectangles overlap with each other or exceed the available range
 * (at y=0), and the combined distance measure of each rectangle from its
 * optimal position is minimized (see below for the distance criterion).
 *
 * Additionally, a spacing can be set. This is the minimum space to keep between
 * adjacent rectangles and between y=0 and the first rectangle.
 *
 * Note that this class does not move anything, widgets or otherwise. It just
 * calculates the positions.
 *
 * To use this class, create an instance and call addItem once for each
 * rectangle i, passing y0_i and h_i. You do not have to keep any particular
 * order; in particular, the items need not be sorted by yo_i. Set the spacing
 * by calling setSpacing and start the optimization by calling doLayout. You can
 * then retrieve the optimized positions by calling items(). The items will be
 * returned in any order, but the originalIndex property will refer to the
 * order in which the items were added to the list.
 *
 * Mathematically, this is an n-dimensional optimization problem with
 * constraints. The target function, which is to be minimized, is the mean
 * square distance from the optimal position:
 *     sum_i ((y_i - yo_i)²)
 * The constraints are
 *     y_0   >= 0         + spacing
 *     y_i+1 >= y_i + h_i + spacing
 * This assumes that the rectangles are sorted by optimal position, i. e.
 *     i<j => yo_i<yo_j,
 * which can be assumed without loss of generality.
 *
 * The constraints form a convex polyhedron in n-dimensional space. We are
 * looking for the point inside this polyhedron that is closest to the optimal
 * position, where y_i = yo_i for all i, which typically is outside of the
 * polyhedron.
 *
 * The heuristic we use for solving this problem is to start with the optimal
 * point and iteratively project it to the farthest unfulfilled constraint, that
 * is, the constraint violated by the biggest margin. In two dimensions, it can
 * be seen that this either results in a point on one edge of the polyhedron or
 * converges towards a corner. It fails only at corners with an angle of more
 * than 90 degrees, but the given constraints seem to produce no such corner.
 *
 * Projecting a point onto a constraint is done:
 *     for y_0  : by setting y_0 = 0 + spacing
 *     for y_i+1: by setting y_i+1 = y_i+1 + delta and y_i = y_i-delta,
 *                where delta is the previous overlap, y_i+h_i+spacing - y_i+1
 *
 * The distance to the constraint is given by the weighted overlap, if positive:
 *     for y_0:   1         * (0+spacing       - y_0)
 *     for y_i+1: sqrt(2)/2 * (y_i+h_i+spacing - y_i+1)
 * The weight factor of sqrt(2)/2=0.71 comes from the fact that the constraint
 * boundaries are not parallel to the coordinate axes. It can also be explained
 * by the fact that moving two rectangles away from their respective ideal
 * positions by (delta/2) each increases the target function by delta²/2,
 * whereas moving one rectangle away from its ideal position by delta increases
 * the target function by delta², which is worse.
 *
 * One such iteration is done by optimizeIteration. We stop after we reach a
 * point inside the polyhedron or after a maximum number of iterations has been
 * reached. Since we may not have reached a valid point yet, we then "force" the
 * constraints by iterating over all rectangles, starting at the first, and
 * moving them down until the constraint is fulfilled. This is done by
 * forceNoOverlaps.
 *
 * Note that this method could probably be extended to constraint angles larger
 * than 90 degrees by choosing the middle between the current point and the
 * ideal point at the end of each iteration.
 *
 * Another, non-heuristic, method would be to use the downhill simplex method,
 * if we can find a way to extend the target function to outside the polyhedron.
 * This would also allow extending the target function to any convex function.
 * One application would be to equally prefer all positions of a notification
 * widget that make the arrow straight (if moved down along the side of the
 * bubble) (not that would make the optimal solution non-unique).
 */
class RectangleLayout
{
	public:
		class Item
		{
			public:
				int originalIndex;
				double y;
				int h;
				bool operator< (const Item &other) const { return y<other.y; }
		};

		RectangleLayout ();
		virtual ~RectangleLayout ();

		void setSpacing (int spacing);

		void addItem (int targetY, int h);
		QList<Item> items ();

		void doLayout (int maxIterations);


	private:
		double overlap (int index) const;
		int largestOverlap () const;

		bool optimizeIteration ();
		void roundPositions ();
		void forceNoOverlaps ();


		int _spacing;
		QList<Item> _items;

};

#endif
