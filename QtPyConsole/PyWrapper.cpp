#include "PyWrapper.h"

PyWrapper::PyWrapper()
{
    std::string stdOutErr = "import sys\n\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
catchOutErr = CatchOutErr()\n\
oldstdout = sys.stdout\n\
sys.stdout = catchOutErr\n\
oldstderr = sys.stderr\n\
sys.stderr = catchOutErr\n\
"; //this is python code to redirect stdouts/stderr

    Py_Initialize();

    pModule = PyImport_AddModule("__main__");                 //create main module
    PyRun_SimpleString(stdOutErr.c_str());                    //invoke code to redirect
    catcher = PyObject_GetAttrString(pModule, "catchOutErr"); //get our catchOutErr created above
    PyErr_Print();                                            //make python print any errors
    output = PyObject_GetAttrString(catcher, "value");        //get the stdout and stderr from our catchOutErr object
}

PyWrapper::~PyWrapper()
{
    PyRun_SimpleString("sys.stdout = oldstdout");
    PyRun_SimpleString("sys.stderr = oldstderr");

    //Py_XDECREF(output);
    //Py_XDECREF(catcher);
    //Py_XDECREF(pModule);

    // Py_FinalizeEx(); //Notice that Py_FinalizeEx() does not free all memory allocated by the Python interpreter,
    // e.g. memory allocated by extension modules currently cannot be released.
    if (Py_FinalizeEx() < 0) exit(120);
    Py_Finalize();
}

bool PyWrapper::isInitialized() const
{
    return Py_IsInitialized() != 0;
}

QString PyWrapper::getOuput()
{
    catcher = PyObject_GetAttrString(pModule, "catchOutErr");
    PyErr_Print();
    output = PyObject_GetAttrString(catcher, "value");
    PyRun_SimpleString("catchOutErr.value = ''");
    return pyObjectToString(output);
}

QString PyWrapper::pyRun(QString command)
{
    QString input = command.trimmed(); //"\"" + command + "\"";
    PyRun_SimpleString("catchOutErr.value = ''");
    PyRun_SimpleString(input.toStdString().c_str());

    catcher = PyObject_GetAttrString(pModule, "catchOutErr"); //get our catchOutErr created above
    PyErr_Print();                                            //make python print any errors

    output = PyObject_GetAttrString(catcher, "value"); //get the stdout and stderr from our catchOutErr object
    return pyObjectToString(output);
}

QString PyWrapper::pyObjectToString(PyObject* poVal)
{
    QString val{};

    if (poVal != NULL)
    {
        if (PyUnicode_Check(poVal))
        {
            // Convert Python Unicode object to UTF8 and return pointer to buffer
            const char* str = PyUnicode_AsUTF8(poVal);

            if (!hasError()) val = QString::fromUtf8(str);
        }

        // Release reference to object
        Py_XDECREF(poVal);
    }

    return val;
}

bool PyWrapper::hasError()
{
    if (PyErr_Occurred())
    {
        // Output error to stderr and clear error indicator
        PyErr_Print();
        return true;
    }
    return false;
}
