@echo off

IF NOT EXIST build mkdir build
pushd build

set DiscardedWarnings=-wd4100
set CompilerFlags=-MTd -GR- -Gm- -EHa- -EHsc- -Od -Oi -W4 %DiscardedWarnings% -FC -Z7
set LinkerFlags=user32.lib Ws2_32.lib d3d11.lib dxgi.lib

set DearImGuiFiles= ^
..\imgui\imgui.cpp ^
..\imgui\imgui_draw.cpp ^
..\imgui\imgui_tables.cpp ^
..\imgui\imgui_widgets.cpp ^
..\imgui\backends\imgui_impl_win32.cpp ^
..\imgui\backends\imgui_impl_dx11.cpp


:: -I --> include ..\imgui directory. Tells the compiler to look into it to to find header files
cl %CompilerFlags% ..\cogne.cpp %DearImGuiFiles% -I..\imgui -Fmcogne.map /link %LinkerFlags%

popd