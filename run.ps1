$project_name=Split-Path -Leaf (Get-Location)
$default_complier="msvc"
$default_build_type="release"
& "./$project_name-$default_complier-$default_build_type.exe"
