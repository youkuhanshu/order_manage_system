#include "FileManager.h"
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
        d_qt.id = d.id;
        d_qt.name = QString::fromStdString(d.name);
        d_qt.price = d.price;
        d_qt.description = QString::fromStdString(d.description);
        d_qt.sales = d.sales;
        d_qt.rating = d.rating;
        d_qt.category = QString::fromStdString(d.category);
        d_qt.comment_count = d.comment_count;
        all_dishes_qt.append(d_qt);

        // 获取菜品种类，用于分类栏
        if (!all_categories_qt.contains(d_qt.category)) {
            all_categories_qt.append(d_qt.category);
        }
    }

    ifs.close();

    auto copy = all_dishes_qt; // 防止原来的向量被修改

    // 按销量降序
    std::sort(copy.begin(), copy.end(), [](const Dish_qt &a, const Dish_qt &b) { return a.sales > b.sales; });
    for (int i = 0; i < qMin(5, copy.size()); i++) {
        recommend_by_sales.append(copy[i]);
    }

    // 按评分降序
    std::sort(copy.begin(), copy.end(), [](const Dish_qt &a, const Dish_qt &b) { return a.rating > b.rating; });
    for (int i = 0; i < qMin(5, copy.size()); i++) {
        recommend_by_rating.append(copy[i]);
    }

    // 按评论数降序
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
        ss >> u.id >> u.name >> u.password >> u.level;
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
    ofs << "\n" << id << " " << name << " " << password << " " << level << std::endl;

    ofs.close();
}

// 加载排队队列
void FileManager::LoadQueue() {
    all_queue_.clear();
    std::ifstream in(QUEUE_FILE_PATH);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        all_queue_.push_back(QueueMsg::from_String(line));
    }
    in.close();
}

// 在C++中使用，更新排队txt
void FileManager::SaveQueue(const std::vector<QueueMsg>& queue) const {
    std::ofstream out(QUEUE_FILE_PATH);
    if (!out) return;
    for (const auto& msg : queue) {
        out << msg.to_String() << "\n";
    }
    out.close();
}

// 在C++中使用，获取全部队列
std::vector<QueueMsg> FileManager::getQueue() {
    return all_queue_;
}

// 加载所有评论
void FileManager::LoadComments() {
    all_comments_.clear();
    std::ifstream in(COMMENT_FILE_PATH);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        all_comments_.push_back(CommentMsg::from_String(line));
    }
    in.close();
}

// 在C++中使用，将评论写入txt
void FileManager::SaveComments(const std::vector<CommentMsg>& comments) const {
    std::ofstream out(COMMENT_FILE_PATH);
    if (!out) return;
    for (const auto& msg : comments) {
        out << msg.to_String() << "\n";
    }
    out.close();
}

// 在C++中使用，获取所有评论
std::vector<CommentMsg> FileManager::getComments() {
    return all_comments_;
}

// 追加一条评论并同步更新 menu.txt 中涉及菜品的评分和评论数
void FileManager::AddCommentAndUpdateMenu(const CommentMsg& comment)
{
    // ---- 1. 追加评论到 comment.txt ----
    {
        std::ofstream out(COMMENT_FILE_PATH, std::ios::app);
        if (out) out << comment.to_String() << "\n";
    }
    all_comments_.push_back(comment);

    // ---- 2. 读取 menu.txt 所有行 ----
    std::vector<std::string> lines;
    {
        std::ifstream in(MENU_FILE_PATH);
        if (!in) return;
        std::string line;
        while (std::getline(in, line)) lines.push_back(line);
    }

    // ---- 3. 对评论中每个菜品，更新对应行的评分和评论数 ----
    for (const auto& dishIdStr : comment.dish_ids) {
        if (dishIdStr.empty()) continue;
        int targetId = std::stoi(dishIdStr);

        for (auto& l : lines) {
            if (l.empty() || l[0] == '#') continue;

            // 按空格分词，格式：id name price desc sales rating category comment_count
            std::vector<std::string> tokens;
            std::istringstream iss(l);
            std::string tok;
            while (iss >> tok) tokens.push_back(tok);
            if (tokens.size() < 8) continue;
            if (std::stoi(tokens[0]) != targetId) continue;

            double  oldRating = std::stod(tokens[5]);
            int     oldCount  = std::stoi(tokens[7]);
            double  newRating = (oldRating * oldCount + comment.rate) / (oldCount + 1);
            int     newCount  = oldCount + 1;

            // 保留一位小数
            std::ostringstream ratingStr;
            ratingStr << std::fixed;
            ratingStr.precision(1);
            ratingStr << newRating;

            tokens[5] = ratingStr.str();
            tokens[7] = std::to_string(newCount);

            // 重组行
            l = tokens[0];
            for (size_t i = 1; i < tokens.size(); i++) l += " " + tokens[i];

            // 同步内存中的 Qt 数据
            for (auto& dq : all_dishes_qt) {
                if (dq.id == targetId) { dq.rating = newRating; break; }
            }
            for (auto& dc : all_dishes_cpp) {
                if (dc.id == targetId) { dc.rating = newRating; break; }
            }
            break;
        }
    }

    // ---- 4. 将更新后的内容写回 menu.txt ----
    std::ofstream out(MENU_FILE_PATH);
    if (!out) return;
    for (const auto& l : lines) out << l << "\n";
}
