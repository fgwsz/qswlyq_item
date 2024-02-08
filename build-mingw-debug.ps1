$project_name=Split-Path -Leaf (Get-Location)
$build_path="./build/mingw/debug"
if(!(Test-Path -Path $build_path)){
    mkdir $build_path
}
cd $build_path
cmake -G "MinGW Makefiles" ../../..
cmake --build .
cd ../../..
cp "$build_path/$project_name.exe" "./$project_name-mingw-debug.exe"
