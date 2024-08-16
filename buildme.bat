echo off
del *.sln
rmdir /s /q .vs
rmdir /s /q Binaries
rmdir /s /q Intermediate
rmdir /s /q Saved

set MyUVS=%UE_VERSION_SELECTOR%
set MyUBT=%UE_BUILD_TOOL%

set MyFullPath=%cd%\cellworld_vr

echo 1
%MyUVS% \projectfiles %MyFullPath%.uproject
echo 2
%MyUBT% Development Win64 -Project=%MyFullPath%.uproject -TargetType=Editor -Progress -NoEngineChanges -NoHotReloadFromIDE
echo 3
%MyFullPath%.uproject
echo 4
%MyFullPath%.sln
echo 5 
echo "done"