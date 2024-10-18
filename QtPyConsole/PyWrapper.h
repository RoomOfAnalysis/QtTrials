#pragma once

// https://docs.python.org/3/extending/embedding.html
// https://github.com/t-cann/qtPythonConsoleCMake

#define PY_SSIZE_T_CLEAN

#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

#include <QString>

class PyWrapper
{
public:
    PyWrapper();
    ~PyWrapper();

    bool isInitialized() const;

    QString getOuput();

    QString pyRun(QString);
    QString pyObjectToString(PyObject*);

private:
    bool hasError();

private:
    PyObject* catcher = nullptr;
    PyObject* output = nullptr;
    PyObject* pModule = nullptr;
};