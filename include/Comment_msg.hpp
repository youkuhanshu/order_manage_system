#include <iostream>
#include <ctime>
#include <vector>
#include <string>

struct CommentMsg{
    std::string user_id;
    std::vector<std::string> dish_ids;  //支持多菜品评论
    std::string comment;
    int rate;
    std::time_t in_time;
    //构造
CommentMsg():user_id(""),comment(""),rate(0),in_time(0){}
CommentMsg(std::string useri,std::string dishi,std::string com,int rate,std::time_t intim):user_id(useri),dish_id(dishi),comment(com),rate(rate),in_time(intim){}

//string和Queue_msg类型转换函数
std::string to_String() const{
    std::string str;
    for(auto it = dish_ids.begin();it != dish_ids.end();it++){
        str += *it + ',';
    }

    return user_id + "|" +
           str + "|" +
           comment + "|" +
           std::to_string(rate) + "|" +
           std::to_string(in_time);
}
static CommentMsg from_String(const std::string& str){
    CommentMsg msg;
    //记录分界点位置
    size_t p1 = str.find('|');
    size_t p2 = str.find('|',p1+1);
    size_t p3 = str.find('|',p2+1);
    size_t p4 = str.find('|',p3+1);
    //若
    if(p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos && p4 != std::string::npos){
        msg.user_id = str.substr(0,p1);
        std::string dish_ids_str = str.substr(p1 + 1,p2 - p1- 1);
        msg.comment = str.substr(p2 + 1,p3 - p2 -1);
        msg.rate = std::stoi(str.substr(p3 + 1,p4 - p3 -1));
        //时间戳用long long更安全
        msg.in_time = std::stoll(str.substr(p4 + 1));

        int len = dish_ids_str.size();
        int last = 0;
        for(int i = 0;i < len;i++){
            if(dish_ids_str[i] == ','){
                msg.dish_ids.push_back(dish_ids_str.substr(last,i - last -1));
                last = i+1;
            }
        }
        return msg;
    }
    //等着加个失败管理
    return;
}
};
