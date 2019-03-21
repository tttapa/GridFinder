#pragma once

#include <GridFinder.hpp>
#include <PyMatrix.hpp>
#include <sstream>

PYBIND11_MODULE(py_grid_finder, pygridmodule) {
    using GM = GridMask<410, 308>;
    pybind11::class_<GM>(pygridmodule, "GridFinder")
        .def(pybind11::init<const GM::Img_t &>())
        .def("getFirstLine", &GM::getFirstLine);

    using GetFirstLineResult = GM::GetFirstLineResult;
    pybind11::class_<GetFirstLineResult>(pygridmodule, "GetFirstLineResult")
        .def("getLineCenter", [](GetFirstLineResult r) { return r.lineCenter; })
        .def("getWidth", [](GetFirstLineResult r) { return r.width; })
        .def("getAngle", [](GetFirstLineResult r) { return r.angle.rad(); });

    pybind11::class_<Pixel>(pygridmodule, "Pixel")
        .def_readwrite("x", &Pixel::x)
        .def_readwrite("y", &Pixel::y)
        .def("__str__", [](Pixel p) {
            std::stringstream s;
            s << p;
            return s.str();
        });
}