#pragma once

#include <GridFinder.hpp>
#include <Line.hpp>
#include <PyMatrix.hpp>
#include <sstream>

PYBIND11_MODULE(py_grid_finder, pygridmodule) {
    using GM = GridMask<410, 308>;
    pybind11::class_<GM>(pygridmodule, "GridFinder")
        .def(pybind11::init<const GM::Img_t &>())
        .def("getFirstLine", &GM::getFirstLine)
        .def("findNextLine", &GM::findNextLine);

    using LineResult = GM::LineResult;
    pybind11::class_<LineResult>(pygridmodule, "LineResult")
        .def("getLineCenter", [](LineResult r) { return r.lineCenter; })
        .def("getWidth", [](LineResult r) { return r.width; })
        .def("getAngle", [](LineResult r) { return r.angle.rad(); });

    pygridmodule.def("intersect", [](LineResult a, LineResult b) {
        Point intersection = Line::intersect(Line(a.lineCenter, a.angle),
                                             Line(b.lineCenter, b.angle));
        return std::make_tuple<int, int>(std::round(intersection.x),
                                         std::round(intersection.y));
    });

    pybind11::class_<Pixel>(pygridmodule, "Pixel")
        .def_readwrite("x", &Pixel::x)
        .def_readwrite("y", &Pixel::y)
        .def("__str__", [](Pixel p) {
            std::stringstream s;
            s << p;
            return s.str();
        });
}