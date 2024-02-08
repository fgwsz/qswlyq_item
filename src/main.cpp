#include"main.hpp"
#include<filesystem>
#include<fstream>
#include<cstdlib>
#include<cctype>
#include<string>
#include<iostream>
#include<regex>
#include<windows.h>

#define ECHO(...) do{ \
    ::std::cout<<#__VA_ARGS__<<":"<<__VA_ARGS__<<::std::endl; \
}while(0)
//
struct HandleResult{
    ::std::string title;
    ::std::string url;
};
::HandleResult handle(::std::string const& input_string);
void set_clipboard(const std::string& text);
void start_url(std::string const& url);
int main(int argc,char** argv){
    auto res_path=::std::filesystem::path(argv[0]).parent_path()/"res";
    auto input_path=res_path/"input.txt";
    ::std::cout<<"res/input.txt as input string\n";
    while(true){
        // 提示用户输入回车开始文本处理
        ::std::cout<<"please press enter to start or press `q` to quit:";
        ::std::string command;
        ::std::getline(::std::cin,command);
        if(command=="q"){
            break;
        }
        // 读入input.txt中的内容到内存变量input_string中
        ::std::ifstream input_file(input_path.string()); // 打开输入文件
        ::std::string input_string;
        if(input_file){
            input_string=::std::string((std::istreambuf_iterator<char>(input_file)),
                                        std::istreambuf_iterator<char>());
            input_file.close();
        }else{
            ::std::cerr<<"input.txt read failed\n";
            return -1;
        }
        ::std::cout<<"input:\n"<<input_string<<::std::endl;
        // 文本处理
        auto result=::handle(input_string);
        ::std::string output_string=result.title+"\n"+result.url+"\n";
        ::std::cout<<"handle result:\n"<<output_string<<::std::endl;
        // 使用默认浏览器打开网址
        ::start_url(result.url);
        ::std::cout<<"open url:\n"<<result.url<<::std::endl;
        // 设置系统剪切板的内容为最终返回值
        ::set_clipboard(output_string);
        ::std::cout<<"copy to clipboard:\n"<<output_string<<::std::endl;
        // 清空input.txt文件中的内容
        ::std::ofstream input_clean(input_path.string(),::std::ios::trunc);
        if(input_clean){
            ::std::cout<<"clean input.txt\n";
            input_clean.close();
        }else{
            ::std::cerr<<"input.txt write failed\n";
            return -1;
        }
    }
    return 0;
}
::std::string get_url(::std::string const& str){
    ::std::regex url_regex(
        R"(https?://[^\s/$.?#].[^\s]*)"
    );
    ::std::smatch match;
    ::std::string url;
    if(::std::regex_search(str,match,url_regex)){
        url=match.str();
    }
    return url;
}
// 删除第一个找到的substr
// 没有找到直接返回::std::string::npos
// 找到之后就返回删除之后的第一个字符下标位置
// 删除后到行尾返回string.size()
::std::size_t string_remove(::std::string& string,::std::string const& substr){
    ::std::size_t pos=string.find(substr);
    if(pos!=string.npos){
        string.erase(pos,substr.length());
    }
    return pos;
}
bool is_space(char ch){
    return ::std::isspace(ch)||ch==' '||ch=='\t'||ch=='\n'||ch=='\r';
}
void string_remove_space(::std::string& string){
    ::std::string new_string={};
    for(auto const& ch:string){
        if(!::is_space(ch)){
            new_string.push_back(ch);
        }
    }
    string=new_string;
}
::HandleResult handle(::std::string const& input_string){
    std::string url = get_url(input_string);
    if (url.empty()){
        std::cerr<<"url not found!\n";
        return {};
    }
    // 把其他内容合并
    ::std::size_t url_begin=input_string.find(url);
    if(url_begin==input_string.npos){
        ::std::cerr<<"find url error!\n";
        return {};
    }
    ::std::size_t url_end=url_begin;
    for(::std::size_t index=url_begin;index<input_string.size();++index){
        if(::is_space(input_string[index])){
            url_end=index;
            break;
        }
    }
    ::std::string title=input_string.substr(0,url_begin)+input_string.substr(url_end,input_string.npos);
    // 删除第一行中的无效信息
    // 要删除的内容：
    // 1.空白字符和换行符
    ::string_remove_space(title);
    // 删除单位信息，以免造成属地信息干扰
    ::string_remove(title,"单位：山东省德州市平原县委宣传部");
    // 查找第一行中的属地信息
    ::std::string const location_list[]={
        "德城",
        "陵城",
        "禹城",
        "乐陵",
        "临邑",
        "平原",
        "夏津",
        "武城",
        "庆云",
        "宁津",
        "齐河",
        "天衢",
        "德州"
    };
    // 如果line_1中含有属地信息那么就将第一行的头部属地信息设置为如下的对应下标的对应值
    // 如果不含有属地信息那么直接赋值为最后一项
    ::std::string const head_list[]={
        "德州德城：",
        "德州陵城：",
        "德州禹城：",
        "德州乐陵：",
        "德州临邑：",
        "德州平原：",
        "德州夏津：",
        "德州武城：",
        "德州庆云：",
        "德州宁津：",
        "德州齐河：",
        "德州天衢：",
        "德州："
    };
    ::std::string title_head="德州：";
    for(::std::size_t index=0;auto const& location:location_list){
        if(title.find(location)!=title.npos){
            title_head=head_list[index];
            break;
        }
        ++index;
    }
    title=title_head+title;
    // 删除第一行中的无效信息
    // 要删除的内容：
    // 2.标题：and 链接：and （如果能找到摘要的话）摘要：以及该行之后的全部信息
    ::string_remove(title,"标题：");
    ::string_remove(title,"链接：");
    do{
        ::std::size_t index=title.find("摘要：");
        if(index!=title.npos){
            title.erase(index,title.npos);
        }
    }while(0);
    // 处理结束返回标题和链接
    return {title,url};
}
void set_clipboard(std::string const& utf8_text){
    // 打开剪贴板
    if (!::OpenClipboard(nullptr)) {
        ::std::cerr<<"open clipboard error!\n";
        return;
    }
    // 清空剪贴板
    ::EmptyClipboard();
    // 将UTF-8文本转换为多字节文本
    int requiredSize=::MultiByteToWideChar(CP_UTF8,0,utf8_text.c_str(),-1,nullptr, 0);
    wchar_t* wideText=new wchar_t[requiredSize];
    ::MultiByteToWideChar(CP_UTF8,0,utf8_text.c_str(),-1,wideText,requiredSize);
    // 分配内存以存储多字节文本
    HGLOBAL hClipboardData=::GlobalAlloc(GMEM_MOVEABLE,requiredSize*sizeof(wchar_t));
    if(hClipboardData != nullptr){
        // 锁定全局内存并获取指向它的指针
        wchar_t* pClipboardData=static_cast<wchar_t*>(::GlobalLock(hClipboardData));
        if (pClipboardData!=nullptr){
            // 将多字节文本复制到全局内存
            ::wcscpy(pClipboardData, wideText);
            // 解锁全局内存
            ::GlobalUnlock(hClipboardData);
            // 将全局内存设置到剪贴板
            ::SetClipboardData(CF_UNICODETEXT,hClipboardData);
        }
    }
    delete[] wideText;
    // 关闭剪贴板
    ::CloseClipboard();
}
std::string string_replace_all(std::string str,std::string const& from,std::string const& to) {
    size_t start_pos=0;
    while((start_pos=str.find(from, start_pos))!=std::string::npos) {
        str.replace(start_pos,from.length(),to);
        start_pos+=to.length();
    }
    return str;
}
void start_url(std::string const& url){
    std::string command="powershell.exe \"Start-Process '"+url+"'\"";
    ::std::system(command.c_str());
}