#


## QtSqliteTest



`Qt` is installed through  `vcpkg` and it already contains `sqlite` plugin.

But this plugin not include `sqlite3` binary, so have to use `cmake` post build command to copy it into the executable folder.


```sh
# before copy sqlite3.dll
qt.core.library: "C:/Users/Harold/source/repos/QtSqliteTest/build/Debug/sqldrivers/qsqlited.dll" cannot load: Cannot load library C:\Users\Harold\source\repos\QtSqliteTest\build\Debug\sqldrivers\qsqlited.dll: The specified module could not be found.
qt.core.plugin.loader: QLibraryPrivate::loadPlugin failed on "C:/Users/Harold/source/repos/QtSqliteTest/build/Debug/sqldrivers/qsqlited.dll" : "Cannot load library C:\\Users\\Harold\\source\\repos\\QtSqliteTest\\build\\Debug\\sqldrivers\\qsqlited.dll: The specified module could not be found."
QSqlDatabase: QSQLITE driver not loaded
QSqlDatabase: available drivers: QSQLITE QPSQL

# after copy sqlite3.dll
qt.core.library: "C:/Users/Harold/source/repos/QtSqliteTest/build/Release/sqldrivers/qsqlite.dll" loaded library
"C:/Users/Harold/source/repos/QtSqliteTest/build/Release/sqldrivers/qsqlite.dll" unloaded library
```
