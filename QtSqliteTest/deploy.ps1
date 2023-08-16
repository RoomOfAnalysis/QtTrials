$mode = $args[0]
$cur_dir = Get-Location

if ($mode -eq "release")
{
    & "C:\vcpkg\installed\x64-windows\tools\Qt6\bin\windeployqt.exe" $cur_dir\build\Release\QtSqliteTest.exe
}
else
{
    & "C:\vcpkg\installed\x64-windows\tools\Qt6\bin\windeployqt.debug.bat" $cur_dir\build\Debug\QtSqliteTest.exe
}
