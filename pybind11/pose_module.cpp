// pose_module.cpp
#include <format>
#include <concepts>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct Pose {
    float x, y, z;
    std::string to_string() const {
        return std::format("({:.3f}, {:.3f}, {:.3f})", x, y, z);
    }
};
static_assert(std::is_trivially_copyable_v<Pose>, "Pose must be trivially copyable");
static_assert(std::is_standard_layout_v<Pose>, "Pose must have standard layout");

// templated function that automatically adds buffer protocol support (as raw byte buffers)
// to any PoD like struct
template <typename T>
    requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
py::class_<T> bind_buffer(py::module_ &m, const char *name)
{
    return py::class_<T>(m, name, py::buffer_protocol())
        .def(py::init<>())
        .def_buffer([](T &self) -> py::buffer_info
                    { return py::buffer_info(
                        // expose as a raw byte buffer
                        reinterpret_cast<uint8_t *>(&self),       // Pointer to buffer
                        1,                                        // size of element in buffer (1 byte for uint8_t)
                        py::format_descriptor<uint8_t>::format(), // Python struct-style format descriptor
                        1,                                        // number of dimensions
                        {sizeof(T)},                              // buffer dimensions
                        {1}                                       // strides (in bytes) for each index
                      ); });
}

PYBIND11_MODULE(pose_module, m) {
    // bind_buffer replaces the usual py::class_ definition
    // to automatically add buffer protocol support for trivially copyable types
    // as raw byte buffers.
    // types defined with `bind_buffer` can be directly read/written using the buffer protocol
    // with (userspace) zero-copy semantics to socket API for example,
    // which is useful for IPC on the same machine.

    bind_buffer<Pose>(m, "Pose")
        .def_readwrite("x", &Pose::x)
        .def_readwrite("y", &Pose::y)
        .def_readwrite("z", &Pose::z) 
        .def("__repr__", &Pose::to_string);
}
