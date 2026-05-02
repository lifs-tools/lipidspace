
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
windeployqt6 --no-translations --compiler-runtime "Build/LipidSpace/LipidSpace.exe"
for /f "delims=" %%i in ('where libgomp-1.dll 2^>nul') do copy "%%i" "Build\LipidSpace\" && goto :gomp_done
:gomp_done
powershell Compress-Archive "Build/LipidSpace" "Build/LipidSpace.zip" -force
