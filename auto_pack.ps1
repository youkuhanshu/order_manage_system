# deploy.ps1 — 一键编译 + 打包
$env:PATH = "D:\Qt\6.9.3\mingw_64\bin;" + $env:PATH

# 重新编译
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
mkdir build; cd build
cmake ../ui -G "MinGW Makefiles"
cmake --build .
if ($LASTEXITCODE -ne 0) { Write-Error "编译失败"; exit 1 }

# 部署 Qt 依赖
windeployqt order_system.exe --qmldir ..\ui\src

# 复制数据文件
mkdir storage\data -Force
copy ..\storage\data\menu.txt          storage\data\
copy ..\storage\data\users.txt         storage\data\
copy ..\storage\data\queue.txt         storage\data\
copy ..\storage\data\comment.txt       storage\data\
copy ..\storage\data\history_order.txt storage\data\

cd ..

# 打包
powershell -Command "Compress-Archive -Path build\order_system.exe,build\Qt6Core.dll,build\Qt6Gui.dll,build\Qt6Network.dll,build\Qt6Svg.dll,build\Qt6Widgets.dll,build\libgcc_s_seh-1.dll,build\libstdc++-6.dll,build\libwinpthread-1.dll,build\opengl32sw.dll,build\D3Dcompiler_47.dll,build\generic,build\iconengines,build\imageformats,build\networkinformation,build\platforms,build\styles,build\tls,build\translations,build\storage -DestinationPath order_system_portable.zip -Force"

Write-Host "✓ 打包完成: order_system_portable.zip" -ForegroundColor Green