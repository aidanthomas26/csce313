#include <iostream>
#include <cstring> //added cstring library

struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

class Shape { //added access modifiers public and private 
    private:
        int vertices;
        Point** points;

    public:
        Shape (int _vertices) {
            vertices = _vertices;
            points = new Point*[vertices+1];
        }

        ~Shape () {
        }

        void addPoints (Point pts[]) { //added pts array parameter that is of an unspecified size
            for (int i = 0; i <= vertices; i++) {
                memcpy(points[i], &pts[i%vertices], sizeof(Point));
            }
        }

        double* area () {
            int temp = 0;
            for (int i = 0; i <= vertices; i++) {
                int lhs = points[i]->x * points[i+1]->y; //used the -> accessor to get the data from the pointer
                int rhs = (*points[i+1]).x * (*points[i]).y; //used the . and dereferencer to get the data from the pointer
                temp += (lhs - rhs);
            }
            double area = abs(temp)/2.0;
            return &area;
        }
};

int main () {
    Point tri1;
    tri1.x = 0;
    tri1.y = 0;

    Point tri2(1,2);

    Point tri3 = Point(2, 0);

    //assigned the values of the structs in 3 differen ways

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape* tri = new Shape(3);
    tri->addPoints(triPts); //used the -> accessor instead of the dot because it was from a pointer

    Point quad1(0, 0); //created the quads using the struct definition
    Point quad2(0, 2);
    Point quad3(2, 2);
    Point quad4(2, 0);

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape* quad = new Shape(4);
    quad->addPoints(quadPts); //used the -> accessor to access the data at the pointer

    // FIXME: print out area of tri and area of quad
    std::cout << tri->area() << std::endl;
    std::cout << quad->area() << std::endl;
}
