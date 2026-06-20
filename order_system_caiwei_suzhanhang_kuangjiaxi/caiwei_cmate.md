# 个人负责功能报告

## 负责模块

| 层 | 模块 | 文件 |
|---|---|---|
| 后端 | QueueService | `include/queue_service.h`、`queue/src/queue_service.cpp` |
| 后端 | CommentService | `include/Comment_service.h`、`comment/src/comment_service.cpp` |
| 数据结构 | QueueMsg / CommentMsg / DishComment_msg | `include/queue_msg.hpp`、`include/Comment_msg.hpp` |
| UI | QueuePage | `ui/src/queue_page.h/cpp` |
| UI | CheckoutDialog | `ui/src/checkout_dialog.h/cpp` |
| UI | CommentDialog | `ui/src/comment_dialog.h/cpp` |

---

## 一、后端设计

### QueueService
维护排队队列和取餐队列
waiting_ (vector) ──advance_queue()──→ taking_ (vector) ──take_meal()──→ 移除
     ↑                                    ↑
  in_queue()                         current_calling++
  分配取餐号(1001起)
  记录时间戳

### CommentService
通过维护排名索引来简化查询过程

```
AddComment(msg)
  ├→ All_Comments_.push_back(msg)        // 时间排序的评论列表
  ├→ UpdateCommentRank(msg)              // 维护 rate_rank_（评分排序索引）
  ├→ UpdateDishComment(msg)              // 维护 Dish_Comments_ map（菜品→评论聚合）
  │     └→ 增量更新 aver_rate（避免每次全量遍历）
  └→ UpdateAllDishRank(msg)              // 维护 all_dish_rate_rank（菜品均分排名）
```

- `getDishComments(id, "rate")` →  取出已排好的列表
- `getDishAverRate(id)` →  读预计算值
- `getBest5Dishs()` → 取排名前 5，含并列处理

### 读写文件序列化

QueueMsg / CommentMsg 各自内嵌 `to_String()` / `from_String()`，`|` 分隔，多菜品 id 用 `,` 分隔。文件读写由 FileManager 调用这些方法。

---

## 二、UI 模块设计

### QueuePage

```
order_system                       QueuePage
    │                                  │
    │  setQueueData(call, w, t, myId)  │
    │ ─────────────────────────────────→│ refreshDisplay()
    │                                  │   ├ 重建 waiting 列表（isMine 高亮）
    │     QTimer 每30s 自刷新           │   ├ 重建 taking 列表
    │                                  │   └ 更新时间文字
    │  pickupRequested(qid)            │
    │ ←─────────────────────────────────│ 用户点"取餐"
```

- 所有数据由 order_system 注入
- `makeTicketRow()` 用 `std::time(nullptr) - msg.in_time` 算等待时长
- QTimer 解决自动叫号链终止后 taking 区时间冻结的问题

### CheckoutDialog

```
order_system::onPickup()
  ├→ CheckoutDialog dlg(names, ids, total, qid, uid, this)
  ├→ dlg.exec()  // 阻塞
  ├→ cm = dlg.getComment()  // 取出 CommentMsg
  └→ if cm.rate > 0: m_fl.AddCommentAndUpdateMenu(cm, m_commentService)
```

- 全部数据构造注入，不碰文件
- `getComment()` 将 dishIds、评分、文字、时间戳打包成 CommentMsg
- 评论写入由 order_system 调用 FileManager 完成

### CommentDialog 

order_system::openCommentDialog(dishId)
  ├→ dish = 查 m_allItems
  ├→ comments = m_commentService.getDishComments(id, "time")  ← 复用后端排序
  └→ CommentDialog dlg(dish, comments, m_users, &m_commentService, this)
        │
        ├ 排序切换时 → loadComments("rate") → cs.getDishComments()
        └ 均分显示 → cs.getDishAverRate()
```

- 排序复用 `rate_rank_` 索引，均分复用 `Dish_Comments_` 预计算值
- 由 MenuPage emit `viewCommentsRequested` → order_system 创建

---

## 三、order_system 中的管理方式

```cpp
// order_system.h 成员
QueueService   m_queueService;      // 排队后端（栈上，全局唯一）
CommentService m_commentService;    // 评论后端（栈上，全局唯一）
QueuePage     *m_queuePage;        // 持久 Widget（QStackedWidget 中）
// CheckoutDialog / CommentDialog 为局部变量，用后即毁
```

三种管理模式：

| 模式 | 模块 |  
|---|---|
| **持久 + 数据更新** | QueuePage | 
| **瞬时 + 获取结果** | CheckoutDialog | 
| **瞬时 + 展示** | CommentDialog  |

信号/槽连接：

```
DishCard::commentClicked → MenuPage emit viewCommentsRequested → order_system::openCommentDialog
QueuePage::pickupRequested → order_system::onPickup
QueuePage::backToMenuRequested → order_system::switchPage(2)
CartPage::checkoutRequested → order_system lambda（入队 + 切排队页）
```

---

## 四、设计决策

1. 文件读写由 FileManager 统一负责,通过 order_system 中介传递数据
2. 前后端分离，后端纯内存运行，前端通过 order_system 中介获取数据

