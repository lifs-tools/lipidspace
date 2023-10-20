if not exist "Build" mkdir "Build"
if not exist "Build/LipidSpace" mkdir "Build/LipidSpace"
if not exist "Build/LipidSpace/data" mkdir "Build/LipidSpace/data"
if not exist "Build/LipidSpace/examples" mkdir "Build/LipidSpace/examples"
copy "libraries/cppgoslin/bin/win64"\* "Build/LipidSpace"
copy "libraries/OpenBLAS/bin/win64"\* "Build/LipidSpace"
copy "libraries/OpenXLSX/bin/win64"\* "Build/LipidSpace"
copy .\data\classes-matrix.csv "Build/LipidSpace/data"
powershell Copy-Item "data/images" -Destination "Build/LipidSpace/data" -Recurse -force
copy .\examples\Example-Dataset.xlsx "Build/LipidSpace/examples"
copy .\examples\ThreeStudies.xlsx "Build/LipidSpace/examples"
copy LICENSE* "Build/LipidSpace"
copy .\release\LipidSpace.exe "Build/LipidSpace"
powershell Copy-Item "libraries/Qt/bin/win64"\* -Destination "Build/LipidSpace" -Recurse -force
powershell Compress-Archive "Build/LipidSpace" "Build/LipidSpace.zip" -force