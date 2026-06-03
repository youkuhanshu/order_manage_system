# 注意事项
1. CMakelists里记得改下路径 
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/mingw_64") # Qt Kit Dir
2. 这一行记得加入系统的mingw32库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets mingw32) # Qt5 Shared Library 
3. 版本要从11到17
set(CMAKE_CXX_STANDARD 17)
4. 在终端临时设置Qt6Core的PATH
# 如果您使用的是 PowerShell (窗口标题栏或命令提示符里有 "PS")
$env:PATH = "D:\Qt\6.9.3\mingw_64\bin;" + $env:PATH


# 使用说明：mkdir build && cd build 
 cmake ../ui -G "MinGW Makefiles"  
 cmake --build
 ./order_system.exe

# 发布程序封装
当您要把程序发给没有安装 Qt 的朋友时，您需要使用 Qt 提供的 windeployqt 工具，它会自动把所有需要的 .dll 文件从 Qt 目录复制到您的 .exe 旁边，打包成一个可以独立运行的文件夹