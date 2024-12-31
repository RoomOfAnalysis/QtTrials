$mode = $args[0]
$project_name = $args[1]
$target_dir = "$(Get-Location)\build\$project_name"
$qt_bin_dir = "D:\vcpkg\installed\x64-windows\tools\Qt6\bin"
$qt_trans_dir = "D:\vcpkg\installed\x64-windows\translations"
$qt_res_dir = "D:\vcpkg\installed\x64-windows\resources"
$release_bin_dir = "D:\vcpkg\installed\x64-windows\bin"
$debug_bin_dir = "D:\vcpkg\installed\x64-windows\debug\bin"
$copy_from_bin = $debug_bin_dir
$copy_to_bin = "$target_dir\Debug"

if ($mode -eq "release") {
    & "$qt_bin_dir\windeployqt.exe" "$target_dir\Release\$project_name.exe"
    $copy_from_bin = $release_bin_dir
    $copy_to_bin = "$target_dir\Release"
}
else {
    & "$qt_bin_dir\windeployqt.debug.bat" "$target_dir\Debug\$project_name.exe"
}

# to support jpeg
Copy-Item "$copy_from_bin\jpeg62.dll" -Destination $copy_to_bin

# to support jp2
Copy-Item "$copy_from_bin\jasper*.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\openjp2.dll" -Destination $copy_to_bin

# to support webp
Copy-Item "$copy_from_bin\libsharpyuv.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\libwebp.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\libwebpdecoder.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\libwebpdemux.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\libwebpmux.dll" -Destination $copy_to_bin

# to support tiff
Copy-Item "$copy_from_bin\liblzma.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\tiff.dll" -Destination $copy_to_bin

# if with ffmpeg
Copy-Item "$copy_from_bin\avcodec-60.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\avdevice-60.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\avfilter-9.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\avformat-60.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\avutil-58.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\swresample-4.dll" -Destination $copy_to_bin
Copy-Item "$copy_from_bin\swscale-7.dll" -Destination $copy_to_bin

# if with sqlite3
Copy-Item "$copy_from_bin\sqlite3.dll" -Destination $copy_to_bin

# if with QtWebEngine
Copy-Item -Path "$qt_trans_dir\*" -Destination "$copy_to_bin\translations" -Recurse
Copy-Item -Path "$qt_res_dir\*" -Destination "$copy_to_bin\resources" -Recurse

# if has translation file
#Copy-Item "$target_dir\$project_name_zh_CN.qm" -Destination "$copy_to_bin\translations"

# if has config.ini
#Copy-Item "$target_dir\config.ini" -Destination $copy_to_bin

