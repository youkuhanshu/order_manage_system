OrderSystem/
├── include/                  # 后端头文件（纯 C++ 逻辑，继承 QObject）
│   ├── data_context.h        
│   ├── menu_mgr.h            
│   ├── order_mgr.h           
│   └── app_mgr.h             
│
├── src/                      # 后端实现文件
│   ├── menu_mgr.cpp          
│   ├── order_mgr.cpp         
│   └── app_mgr.cpp           
│
├── ui/                       # Qt 前端界面（建议使用 Qt Designer 拖拽生成）
│   ├── mainwindow.h / .cpp / .ui  # 主窗口（包含 TabWidget 切换各个子页面）
│   ├── menu_view.h / .cpp / .ui   # 同学A：菜单展示页
│   ├── cart_view.h / .cpp / .ui   # 同学B：购物车与结算页
│   └── queue_view.h / .cpp / .ui  # 同学C：排队与评价页
│
├── main.cpp                  # 程序入口：初始化 Qt 和后端对象
└── CMakeLists.txt            # CMake 构建脚本（必须配置 AUTOMOC）
