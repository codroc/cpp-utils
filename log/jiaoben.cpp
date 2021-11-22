#include <stdio.h>
#include <Python.h>
int main(int argc, char *argv[]) {
    char *filename = argv[1];
    // printf("going to exec jiaoben.py filename = %s\n", filename);
    Py_Initialize();

    PyObject* pModule = NULL;
    PyObject* pFunc = NULL;
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    pModule = PyImport_ImportModule("jiaoben");
    pFunc = PyObject_GetAttrString(pModule, "TarLogFile");

    PyObject* pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", filename));

    PyEval_CallObject(pFunc, pArgs);

    Py_Finalize();
    return 0;
}
