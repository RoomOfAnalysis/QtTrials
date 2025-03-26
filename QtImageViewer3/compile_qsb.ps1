$qsb = "D:\vcpkg\installed\x64-windows\tools\Qt6\bin\qsb.exe"

& $qsb --glsl "150,120,100 es" --hlsl 50 --msl 12 -o ./shader/vulkan.vert.qsb ./shader/vulkan.vert
& $qsb --glsl "150,120,100 es" --hlsl 50 --msl 12 -o ./shader/vulkan.frag.qsb ./shader/vulkan.frag
