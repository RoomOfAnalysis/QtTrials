#pragma once

#include "PyWrapper.h"

// https://docs.python.org/3/library/code.html
class PyInteractiveWrapper
{
public:
    PyInteractiveWrapper();
    ~PyInteractiveWrapper();

    PyObject* getInteractiveConsoleObj() const;
    PyObject* getInteractiveConsoleLocalsObj() const;

    // code.InteractiveConsole.push(line)
    // https://docs.python.org/3/library/code.html#code.InteractiveConsole.push
    // Push a line of source text to the interpreter.
    std::pair<bool, QString> push(QString);

    // run script with console locals
    std::pair<bool, QString> pushScript(QString);

    void reset();

private:
    PyObject* getConsole();

private:
    PyWrapper* wrapper = nullptr;
    PyObject* interactiveConsole = nullptr;
    PyObject* interactiveConsoleLocals = nullptr;
};