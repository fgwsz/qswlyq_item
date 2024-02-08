function rmrf($path) { Remove-Item $path -Recurse -Force }
rmrf ./build/
rmrf ./*.exe
