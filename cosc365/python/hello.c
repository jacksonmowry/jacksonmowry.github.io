#include <Python.h>

static PyObject *say_hello(PyObject *self, PyObject *args) {
  return Py_BuildValue("s", "Hello from C!");
}

static PyMethodDef MyModuleMethods[] = {
    {"say_hello", say_hello, METH_NOARGS, "Say hello!!"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef mymodule = {PyModuleDef_HEAD_INIT, "hello", NULL, -1,
                                      MyModuleMethods};

PyMODINIT_FUNC PyInit_hello(void) { return PyModule_Create(&mymodule); }
