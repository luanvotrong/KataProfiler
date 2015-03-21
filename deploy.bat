if exist _build rd /s /q _build

md _build
md _build\android
md _build\android\armeabi-v7a
md _build\android\armeabi
md _build\android\x86

copy Client\bin\Release\KataProfiler.exe _build
copy Client\bin\Release\*.dll _build

copy Server\projects\android\release\armeabi-v7a\kataprofiler\libkataprofiler.a _build\android\armeabi-v7a
copy Server\projects\android\release\armeabi\kataprofiler\libkataprofiler.a _build\android\armeabi
copy Server\projects\android\release\x86\kataprofiler\libkataprofiler.a _build\android\x86

copy readme_release.txt _build