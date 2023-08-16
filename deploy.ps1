$mode = $args[0]
$project_name = $args[1]
$cur_dir = Get-Location

if ($mode -eq "release")
{
    & "C:\vcpkg\installed\x64-windows\tools\Qt6\bin\windeployqt.exe" $cur_dir\build\$project_name\Release\$project_name.exe
}
else
{
    & "C:\vcpkg\installed\x64-windows\tools\Qt6\bin\windeployqt.debug.bat" $cur_dir\build\$project_name\Debug\$project_name.exe
}
