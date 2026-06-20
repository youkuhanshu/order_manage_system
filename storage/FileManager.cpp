#include "FileManager.h"
#include "Comment_service.h"
#include <algorithm>

std::vector<Dish> FileManager::all_dishes_cpp;
QList<Dish_qt> FileManager::all_dishes_qt;
QStringList FileManager::all_categories_qt;
QList<Dish_qt> FileManager::recommend_by_sales;
QList<Dish_qt> FileManager::recommend_by_rating;
QList<Dish_qt> FileManager::recommend_by_comments;

std::vector<User> FileManager::all_users_cpp;

std::vector<QueueMsg> FileManager::all_queue_;
std::vector<CommentMsg> FileManager::all_comments_;

// 加载全部菜品
void FileManager::LoadMenu() {
    all_dishes_cpp.clear();
    all_dishes_qt.clear();
    all_categories_qt.clear();
    recommend_by_sales.clear();
    recommend_by_rating.clear();
    recommend_by_comments.clear();

    std::string line;
    std::ifstream ifs;
    ifs.open(MENU_FILE_PATH, std::ios::in);

    if ( !ifs.is_open() ) {
        std::cout << "无法打开菜单文件" << std::endl;
        return;
    }

    while (getline(ifs, line)) { // 从文件流中读取一整行文本，直到遇到换行符或文件结束
        if (line.empty() || line[0] == '#') continue;

        Dish d;
        Dish_qt d_qt;
        std::stringstream ss(line);

        // 将菜品填入vector
        ss >> d.id >> d.name >> d.price >> d.description >> d.sales >> d.rating >> d.category >> d.comment_count;
        all_dishes_cpp.push_back(d);

        // 将菜品填入QList
        d_qt = dish_to_qt(d);
        all_dishes_qt.append(d_qt);

        // 获取菜品种类，用于分类栏
        if (!all_categories_qt.contains(d_qt.category)) {
            all_categories_qt.append(d_qt.category);
        }
    }

    ifs.close();

    auto copy = all_dishes_qt; // 防止原来的向量被修改

    // sort会根据lambda返回的结果决定谁在前谁在后
    // 按销量
    std::sort(copy.begin(), copy.end(), [](const Dish_qt &a, const Dish_qt &b) { return a.sales > b.sales; });
    for (int i = 0; i < qMin(5, copy.size()); i++) {
        recommend_by_sales.append(copy[i]);
    }

    // 按评分
    std::sort(copy.begin(), copy.end(), [](const Dish_qt &a, const Dish_qt &b) { return a.rating > b.rating; });
    for (int i = 0; i < qMin(5, copy.size()); i++) {
        recommend_by_rating.append(copy[i]);
    }

    // 按评论数
    std::sort(copy.begin(), copy.end(), [](const Dish_qt &a, const Dish_qt &b) { return a.comment_count > b.comment_count; });
    for (int i = 0; i < qMin(5, copy.size()); i++) {
        recommend_by_comments.append(copy[i]);
    }
}

// 在C++中使用，获取全部菜品
std::vector<Dish> FileManager::getMenu_cpp() {
    return all_dishes_cpp;
}

// 在Qt中使用
QList<Dish_qt> FileManager::getMenu_qt() {
    return all_dishes_qt;
}

// 在Qt中使用
QStringList FileManager::getCategories_qt() {
    return all_categories_qt;
}

// 在Qt中使用，按销量排序推荐
QList<Dish_qt> FileManager::getRecommendBySales() {
    return recommend_by_sales;
}

// 在Qt中使用，按评分排序推荐
QList<Dish_qt> FileManager::getRecommendByRating() {
    return recommend_by_rating;
}

// 在Qt中使用，按评论数排序推荐
QList<Dish_qt> FileManager::getRecommendByComments() {
    return recommend_by_comments;
}

// 加载全部用户
void FileManager::LoadUsers() {
    all_users_cpp.clear();

    std::string line;
    std::ifstream ifs;
    
    ifs.open(USER_FILE_PATH, std::ios::in);

    if ( !ifs.is_open() ) {
        std::cout << "无法打开用户文件" << std::endl;
        return;
    }

    while (getline(ifs, line)) { // 从文件流中读取一整行文本，直到遇到换行符或文件结束
        if (line.empty() || line[0] == '#') continue; // 跳过空行和注释

        User u;
        std::stringstream ss(line);

        // 将用户填入vector
        ss >> u.id >> u.name >> u.password >> u.level >> u.total_spent;
        all_users_cpp.push_back(u);
    }

    ifs.close();
}

// 在C++中使用，获取全部用户
std::vector<User> FileManager::getUsers_cpp() {
    return all_users_cpp;
}

// 在C++中使用，添加用户
void FileManager::addUser(int id, std::string name, std::string password) {
    std::ofstream ofs;
    int level = 0;

    ofs.open(USER_FILE_PATH, std::ios::app);
    ofs << "\n" << id << " " << name << " " << password << " " << level;

    ofs.close();
}

// 加载排队队列
void FileManager::LoadQueue() {
    all_queue_.clear();

    std::ifstream ifs;
    std::string line;
    
    ifs.open(QUEUE_FILE_PATH, std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "无法打开排队文件" << std::endl;
        return;
    }
    
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        all_queue_.push_back(QueueMsg::from_String(line));
    }

    ifs.close();
}

// 在C++中使用，更新排队txt
void FileManager::SaveQueue(const std::vector<QueueMsg>& queue) const {

    std::ofstream ofs;

    ofs.open(QUEUE_FILE_PATH, std::ios::out);
    
    if (!ofs.is_open()) {
        std::cout << "无法打开排队文件" << std::endl;
        return;
    }
    
    for (const auto& msg : queue) {
        ofs << msg.to_String() << "\n";
    }

    ofs.close();
}

// 在C++中使用，获取全部队列
std::vector<QueueMsg> FileManager::getQueue() {
    return all_queue_;
}

// 加载所有评论
void FileManager::LoadComments() {
    all_comments_.clear();

    std::ifstream ifs;
    std::string line;

    ifs.open(COMMENT_FILE_PATH, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "无法打开评论文件" << std::endl;
        return;
    }
    
    while (std::getline(ifs, line)) {
        if (line.empty() || line[0] == '#') continue;
        all_comments_.push_back(CommentMsg::from_String(line));
    }

    ifs.close();
}

// 在C++中使用，将评论写入txt
void FileManager::SaveComments(const std::vector<CommentMsg>& comments) const {

    std::ofstream ofs;
    
    ofs.open(COMMENT_FILE_PATH, std::ios::out);

    if (!ofs.is_open()) {
        std::cout << "无法打开评论文件" << std::endl;
        return;
    }

    for (const auto& msg : comments) {
        ofs << msg.to_String() << "\n";
    }

    ofs.close();
}

// 在C++中使用，获取所有评论
std::vector<CommentMsg> FileManager::getComments() {
    return all_comments_;
}

// 追加与更新评论并同步到 menu.txt
void FileManager::AddCommentAndUpdateMenu(const CommentMsg& comment, CommentService& cs)
{
    // 1. 追加评论到 comment.txt
    std::ofstream ofs;
    ofs.open(COMMENT_FILE_PATH, std::ios::app);
    if (!ofs.is_open()) {
        std::cout << "无法打开评论文件" << std::endl;
        return;
    }
    ofs << comment.to_String() << "\n";
    ofs.close();
    all_comments_.push_back(comment);

    // 2. 完成评分更新
    cs.AddComment(comment);

    // 3. 从 CommentService 同步评分和评论数到内存中的菜品数据
    for (const auto& dishIdStr : comment.dish_ids) {
        if (dishIdStr.empty()) continue;
        int targetId = std::stoi(dishIdStr);

        double newRating = cs.getDishAverRate(dishIdStr);

        // 同步到 all_dishes_cpp
        for (auto& dish : all_dishes_cpp) {
            if (dish.id == targetId) {
                dish.rating = newRating;
                dish.comment_count++;
                break;
            }
        }

        // 同步到 all_dishes_qt
        for (auto& dish_qt : all_dishes_qt) {
            if (dish_qt.id == targetId) {
                dish_qt.rating = newRating;
                dish_qt.comment_count++;
                break;
            }
        }
    }

    // 4. 将更新后的 all_dishes_cpp 写回 menu.txt
    std::ofstream out;
    out.open(MENU_FILE_PATH, std::ios::out);
    if (!out.is_open()) {
        std::cout << "无法写入菜单文件" << std::endl;
        return;
    }
    for (const auto& dish : all_dishes_cpp) {
        out << dish.id << " " << dish.name << " " << dish.price << " "
            << dish.description << " " << dish.sales << " ";
        out.precision(1);
        out << std::fixed << dish.rating << " " << dish.category << " "
            << dish.comment_count << "\n";
    }
    out.close();
}

// 类型转换
Dish_qt FileManager::dish_to_qt(Dish& dish_cpp) {
    Dish_qt d;
    d.id = dish_cpp.id;
    d.name = QString::fromStdString(dish_cpp.name);
    d.price = dish_cpp.price;
    d.description = QString::fromStdString(dish_cpp.description);
    d.sales = dish_cpp.sales;
    d.rating = dish_cpp.rating;
    d.category = QString::fromStdString(dish_cpp.category);
    d.comment_count = dish_cpp.comment_count;
    return d;
}

Dish FileManager::dish_to_cpp(const Dish_qt& dish_qt) {
    Dish d;
    d.id = dish_qt.id;
    d.name = dish_qt.name.toStdString();
    d.price = dish_qt.price;
    d.description = dish_qt.description.toStdString();
    d.sales = dish_qt.sales;
    d.rating = dish_qt.rating;
    d.category = dish_qt.category.toStdString();
    d.comment_count = dish_qt.comment_count;
    return d;
}

User_qt FileManager::user_to_qt(const User& user_cpp) {
    User_qt u;
    u.id = user_cpp.id;
    u.name = QString::fromStdString(user_cpp.name);
    u.password = QString::fromStdString(user_cpp.password);
    u.level = QString::fromStdString(user_cpp.level);
    u.total_spent = user_cpp.total_spent;
    return u;
}

User FileManager::user_to_cpp(const User_qt& user_qt) {
    User u;
    u.id = user_qt.id;
    u.name = user_qt.name.toStdString();
    u.password = user_qt.password.toStdString();
    u.level = user_qt.level.toStdString();
    u.total_spent = user_qt.total_spent;
    return u;
}

QueueMsg FileManager::queuemsg_to_cpp(const QueueMsg_qt& q) {
    QueueMsg m;
    m.queue_id = q.queue_id;
    m.order_id = q.order_id;
    m.in_time = static_cast<std::time_t>(q.in_time.toSecsSinceEpoch());
    return m;
}

QueueMsg_qt FileManager::queuemsg_to_qt(const QueueMsg& msg) {
    QueueMsg_qt q;
    q.queue_id = msg.queue_id;
    q.order_id = msg.order_id;
    q.in_time = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(msg.in_time));
    return q;
}

CommentMsg FileManager::commentmsg_to_cpp(const CommentMsg_qt& q) {
    CommentMsg m;
    m.user_id = q.user_id.toStdString();
    for (const auto &s : q.dish_ids) m.dish_ids.push_back(s.toStdString());
    m.comment = q.comment.toStdString();
    m.rate = q.rate;
    m.in_time = static_cast<std::time_t>(q.in_time.toSecsSinceEpoch());
    return m;
}

CommentMsg_qt FileManager::commentmsg_to_qt(const CommentMsg& msg) {
    CommentMsg_qt q;
    q.user_id = QString::fromStdString(msg.user_id);
    for (const auto &s : msg.dish_ids) q.dish_ids.append(QString::fromStdString(s));
    q.comment = QString::fromStdString(msg.comment);
    q.rate = msg.rate;
    q.in_time = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(msg.in_time));
    return q;
}

DishComment_msg FileManager::dishcomment_to_cpp(const DishComment_msg_qt& q) {
    DishComment_msg d;
    d.aver_rate = q.aver_rate;
    d.dish_comment_num = q.dish_comment_num;
    for (const auto &c : q.comments) d.comments.push_back(commentmsg_to_cpp(c));
    for (int r : q.rate_rank) d.rate_rank.push_back(r);
    return d;
}

DishComment_msg_qt FileManager::dishcomment_to_qt(const DishComment_msg& msg) {
    DishComment_msg_qt q;
    q.aver_rate = msg.aver_rate;
    q.dish_comment_num = msg.dish_comment_num;
    for (const auto &c : msg.comments) q.comments.append(commentmsg_to_qt(c));
    for (int r : msg.rate_rank) q.rate_rank.append(r);
    return q;
}
