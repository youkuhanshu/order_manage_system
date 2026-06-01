# 点单管理系统 - 期末小组项目分工与开发指南

> 📅 **适用场景**：C++ 期末 3 人小组作业 | 控制台交互 | 零第三方依赖 | 快速联调
> 🎯 **设计原则**：数据结构统一、接口扁平、一人一模块、半天对齐、一天联调

---

## 📁 极简项目目录结构

```text
OrderSystem/
├── common.h          # 全局类型、结构体、共享数据声明（三人共用契约）
├── menu_mgr.h / .cpp # 同学A：菜单管理 & 推荐逻辑
├── order_mgr.h / .cpp# 同学B：订单结算 & 会员折扣 & 历史复制
├── app_mgr.h / .cpp  # 同学C：评价系统 & 排队显示 & 主控交互
└── main.cpp          # 程序入口（仅初始化+调用 run()）


#pragma once
#include <string>
#include <vector>
#include <ctime>

using ID = int;
using Price = int; // 统一用“分”存储，避免浮点误差（显示时 /100.0）

enum Level { REGULAR, SILVER, GOLD, PLATINUM };
enum Status { WAITING, PREPARING, READY };

struct Dish {
    ID id; 
    std::string name, desc, category; 
    Price price;
    int sales = 0; 
    double avg_rating = 0.0; 
    int rating_cnt = 0;
    bool is_top5 = false; // 功能四：推荐标记
};

struct Review { 
    ID dish_id; 
    int star; 
    std::string msg; 
    time_t time; 
};

struct CartItem { 
    ID dish_id; 
    int count; 
};

struct Order {
    ID id; 
    std::vector<CartItem> items; 
    Price final_price;
    Level member_lvl; 
    time_t create_time;
};

// 全局共享数据容器
struct SysData {
    std::vector<Dish> dishes;
    std::vector<Review> reviews;
    std::vector<Order> history;
    std::vector<Order> queue; // 排队队列
    int next_dish_id = 1;
    int next_order_id = 1000;
};

// 全局变量声明（仅在一个 .cpp 中定义，其他文件 extern）
extern SysData g_data; 


👥 三人任务拆分明细
👨‍💻 同学 A

    负责文件：menu_mgr.h / menu_mgr.cpp
    对应功能：功能一（菜单创建与分类）、功能四（推荐点餐）
    核心接口：addDish(), printMenu(bool show_top5), updateTop5()
    交付要求：菜单必须按分类打印并显示说明；结算后能正确调用 updateTop5() 标记销量和评价最好的前 5 个菜品。

👨‍💻 同学 B

    负责文件：order_mgr.h / order_mgr.cpp
    对应功能：功能二（价格计算与会员折扣）、功能五（点餐记忆/历史复制）
    核心接口：calcCart(), checkout(), getDiscountRate(), cloneOrder(), addToHistory()
    交付要求：折扣计算必须准确无误；能够通过历史订单 ID 一键复制出当时的购物车明细。

👨‍💻 同学 C

    负责文件：app_mgr.h / app_mgr.cpp 以及 main.cpp
    对应功能：功能三（点餐评价与排序）、功能六（排队实时显示）、功能七（系统界面与主控）
    核心接口：addReview(), getSortedReviews(), enqueueOrder(), printQueueRealtime(), run()
    交付要求：主控制台循环流畅不卡死；评价严格按规则排序；排队界面刷新逻辑合理。
    
    
 🛠️ 关键功能实现约定
1. 菜单分类

    实现规则：遍历 g_data.dishes，按 category 字段分组打印，支持显示菜品的详细说明 desc。
    负责人：同学 A

2. 会员折扣

    实现规则：设定基础折扣率（如 REGULAR=1.0, SILVER=0.95, GOLD=0.85, PLATINUM=0.75）。消费额累加自动升级（如每消费 10000 分升一级）。
    负责人：同学 B

3. 评价排序

    实现规则：先按 star 降序排列，如果星级相同则按 time 降序排列。必须使用 std::sort 配合 Lambda 表达式实现。
    负责人：同学 C

4. 推荐 Top5

    实现规则：计算综合分（公式：sales * 0.6 + avg_rating * 10 * 0.4）。排名前 5 的菜品设置 is_top5 = true，菜单打印时追加 ⭐推荐 标识。
    负责人：同学 A

5. 订单记忆

    实现规则：遍历 g_data.history 匹配传入的 order_id，找到后返回其 items 副本，供用户直接重新结算。
    负责人：同学 B

6. 排队刷新

    实现规则：使用 system("cls") (Windows) 或 system("clear") (Mac/Linux) 进行清屏重绘。可配合 sleep 函数模拟进度的实时推进。
    负责人：同学 C

7. 系统界面

    实现规则：使用 while(true) 构建主菜单循环，通过数字选项路由到对应功能。负责整合 A 和 B 的接口并处理用户的异常输入。
    负责人：同学 C


🔄 联调与协作流程

    Day 1 上午：三人共同确认 common.h 的数据结构，锁定后严禁任何人私自修改。
    Day 1 下午：各自在独立的文件或分支中开发，只允许依赖 common.h 和 g_data，禁止 A 和 B 互相 include 头文件。
    Day 2 上午：同学 C 创建 main.cpp，实例化三个 Manager 类，编写主菜单路由框架。
    Day 2 下午：全员联调测试。按以下顺序走通全流程：添加菜品 → 点单结算 → 查看排队 → 提交评价 → 复制历史订单 → 查看推荐菜单。
    Git 协作建议：
        分支策略：建立 main (稳定版) 以及 dev-a / dev-b / dev-c。
        每日合并到 main 前，务必先执行 git pull origin main 解决潜在冲突。

✅ 最终验收 Checklist

    编译无 multiple definition 错误（确保 g_data 仅在一个 cpp 中定义，其他文件使用 extern）。
    所有价格输入和存储均使用 int（分），显示时格式化为 x.xx 元。
    评价列表严格按照 星级↓ → 时间↓ 的顺序排列显示。
    结算后 updateTop5() 成功生效，下次打印菜单时能正确显示 ⭐推荐 标识。
    排队界面刷新时不闪屏、不卡死，状态推进逻辑符合预期。
    历史订单复制后，能无缝直接进入结算流程。
    代码包含基础注释，变量命名规范统一，无硬编码的魔法数字。

💡 防坑指南（期末项目高频问题）
g_data 报多重定义 (Multiple Definition)

    解决方案：在 menu_mgr.cpp (或任意一个 cpp) 的首行写 SysData g_data; 进行实体化，其他所有文件只保留 extern SysData g_data; 进行声明。

浮点价格精度丢失 (如 0.1+0.2!=0.3)

    解决方案：全程使用 int 存储“分”，输出时使用 printf("%.2f", price/100.0) 或 cout << fixed << setprecision(2) 进行格式化。

控制台清屏跨平台报错

    解决方案：使用宏判断操作系统：#ifdef _WIN32 system("cls"); #else system("clear"); #endif。

排序结果不稳定或程序崩溃

    解决方案：std::sort 的 Lambda 表达式必须满足严格弱序，当两个元素相等时，必须返回 false。

模块互相调用导致死锁或循环依赖

    解决方案：A 和 B 绝不互相 include 对方的头文件。所有交叉逻辑（例如结算后更新菜品销量）必须由 C 在 run() 主循环中串联调用。

