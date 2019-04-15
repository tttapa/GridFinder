#include <GridFinder.hpp>
#include "TestMask.hpp"

int main() {
    GridFinder gf = {mask};
    cout << gf.findSquare() << endl;
}