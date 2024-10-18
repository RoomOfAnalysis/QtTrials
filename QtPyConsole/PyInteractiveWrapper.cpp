#include "PyInteractiveWrapper.h"

#include "Python.h"

#include <QDebug>

class __PyGilEnsurer
{
public:
    __PyGilEnsurer() { state = PyGILState_Ensure(); }

    ~__PyGilEnsurer() { PyGILState_Release(state); }

private:
    PyGILState_STATE state = PyGILState_STATE::PyGILState_UNLOCKED;
};

PyInteractiveWrapper::PyInteractiveWrapper(): wrapper(new PyWrapper) {}

PyInteractiveWrapper::~PyInteractiveWrapper()
{
    reset();
    delete wrapper;
}

PyObject* PyInteractiveWrapper::getInteractiveConsoleObj() const
{
    return interactiveConsole;
}

PyObject* PyInteractiveWrapper::getInteractiveConsoleLocalsObj() const
{
    return interactiveConsoleLocals;
}

std::pair<bool, QString> PyInteractiveWrapper::push(QString line)
{
    auto* console = getConsole();

    if (!console) return {};

    line.replace("\r\n", "\n");
    line.replace("\r", "\n");
    // trim tailling newline only
    while (line.back() == '\n')
        line.removeLast();

    bool ret_value = false;
    QString outstring;
    {
        __PyGilEnsurer _;
        if (PyObject* res = PyObject_CallMethod(console, "push", "z", line.toUtf8().data()); res)
        {
            int status = 0;
            if (PyArg_Parse(res, "i", &status)) ret_value = (status > 0);
            Py_DECREF(res);
        }
    }
    if (!ret_value) outstring = wrapper->getOuput();
    outstring = outstring.trimmed();
    //qDebug() << "\"" << line << "\"" << ret_value << outstring;
    return std::make_pair(ret_value, outstring);
}

std::pair<bool, QString> PyInteractiveWrapper::pushScript(QString script)
{
    auto* console = getConsole();

    if (!console || script.isEmpty()) return {};

    script.replace("\r\n", "\n");
    script.replace("\r", "\n");

    bool ret_value = false;
    QString outstring;
    {
        __PyGilEnsurer _;
        // https://docs.python.org/3/c-api/veryhigh.html#c.PyRun_String
        if (PyObject* result =
                PyRun_String(script.toUtf8().data(), Py_file_input, interactiveConsoleLocals, interactiveConsoleLocals);
            result)
        {
            Py_DECREF(result);
            if (PyObject* f = PySys_GetObject("stdout"); f == nullptr || PyFile_WriteString("\n", f) != 0)
                PyErr_Clear();
            ret_value = true;
        }
        outstring = wrapper->getOuput();
    }
    return std::make_pair(ret_value, outstring);
}

void PyInteractiveWrapper::reset()
{
    if (interactiveConsole)
    {
        __PyGilEnsurer _;
        Py_XDECREF(interactiveConsoleLocals);
        Py_XDECREF(interactiveConsole);
        interactiveConsole = nullptr;
        interactiveConsoleLocals = nullptr;
        if (wrapper->isInitialized())
        {
            const char* code = "import gc; gc.collect()\n";
            wrapper->pyRun(code);
        }
    }
}

PyObject* PyInteractiveWrapper::getConsole()
{
    if (!interactiveConsole)
    {
        __PyGilEnsurer _;
        // set up the code.InteractiveConsole instance
        const char* code = "import code\n"
                           "__qtPyConsoleLocals={'__name__':'__qtpyconsole__','__doc__':None}\n"
                           "__qtPyConsole=code.InteractiveConsole(__qtPyConsoleLocals)\n";

        PyRun_SimpleString(code);

        // get the reference to __qtPyConsole and save the pointer
        PyObject* main_module = PyImport_AddModule("__main__");
        PyObject* global_dict = PyModule_GetDict(main_module);
        interactiveConsole = PyDict_GetItemString(global_dict, "__qtPyConsole");
        interactiveConsoleLocals = PyDict_GetItemString(global_dict, "__qtPyConsoleLocals");
        if (!interactiveConsole || !interactiveConsoleLocals)
            qFatal() << "Failed to locate the InteractiveConsole/InteractiveConsoleLocals object.";
    }
    return interactiveConsole;
}
