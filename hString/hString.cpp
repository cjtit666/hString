
#include <iostream>
#include <cstring>

class hstring {
private:
    static const unsigned int INIT_BUFF_SIZE = 128; //默认缓冲区大小
    char stack_buf[INIT_BUFF_SIZE];                 //默认缓冲区字符数组
    char* buffer;                                   //动态缓冲区（含初始缓冲区）
    unsigned int buffer_size;                       //缓冲区总容量
    unsigned int data_length;                       //实际存储字符串长度(不包含末尾0)

    //扩容函数
    bool expand(unsigned need) {
        if (need <= buffer_size) { //和当前的缓冲区大小比较
            return true; //不需要扩容
        }
        else {
            unsigned new_size = buffer_size;
            while (new_size < need) {
                new_size *= 2; //不够默认扩容两倍
            }
            char* new_buf = new char[new_size];
            memset(new_buf, 0, new_size);
            memcpy(new_buf, buffer, data_length);
            new_buf[data_length] = '\0';

            if (buffer != stack_buf) {
                delete[] buffer; //释放原本的缓冲区,只能释放堆中new出来的区域
            }
            buffer=new_buf;
            buffer_size = new_size;
            return true;
        }
    }


public:
    //初始化类的变量
    void init_hstring();
    //构造函数
    hstring();
    hstring(const char* str);
    //副本构造函数
    hstring(const hstring& other);

    //析构函数
    ~hstring() {
        if (buffer != stack_buf) {
            delete[] buffer; //释放堆中new出来的区域,不写的话存在内存泄漏
        }
    }

    //赋值运算符重载 支持int转hstring
    hstring& operator = (const hstring& other);
    hstring& operator = (const char* str);
    hstring& operator = (const int num);

    //重载+ 拼接hstring+hstring / hstring+const char*
    hstring& operator+(const hstring& other);
    hstring& operator+(const char* sub);

    //重载- 子串删除 删除第一次出现的子串 未找到时报错 返回删除结果
    hstring& operator-(const hstring& sub);
    hstring& operator-(const char* sub);

    //修改替换操作
    bool replace(unsigned index, const char* A, const char* B);

    //int转str
    void int_to_str(int num, char* result, unsigned& len);

    //导出字符串
    const char* c_str() const {
        return buffer;
    }

    //查找子串
    int find_sub(const char* list1, unsigned len1, const char* list2, unsigned l2);

    //查找返回子串初始索引
    int find(const char* context);


    //获取长度
    unsigned int length() const {
        return data_length;
    }

    //打印字符串
    void printstr() {
        for (int i = 0; i < data_length; i++) {
            std::cout << buffer[i];
        }
        return;
    }

    

};

void hstring::init_hstring()
{
    buffer = stack_buf;
    buffer_size = INIT_BUFF_SIZE;
    data_length = 0;
    memset(stack_buf, 0, INIT_BUFF_SIZE);
}


//-----构造函数的定义-----
hstring::hstring() {
    init_hstring();              //默认全写0
}
hstring::hstring(const char* str) {
    init_hstring();

    if (str == nullptr) { //输入为空
        return;
    }

    //计算字符串长度
    unsigned len = 0;
    while (str[len] != '\0') {
        len++;
    }
    
    //判断是否扩容
    expand(len+1); //需要多放一个'\0'

    data_length = len;

    //拷贝新数据进去
    memcpy(buffer, str, len);
    buffer[len] = '\0';

}
hstring::hstring(const hstring& other) {
    init_hstring();

    //拷贝
    expand(other.data_length + 1);
    data_length = other.data_length;
    memcpy(buffer, other.buffer, data_length);
    buffer[data_length] = '\0';
}


//赋值运算符重载
hstring& hstring::operator=(const hstring& other)
{
    if (this == &other) {
        return *this;
    }

    if (buffer != stack_buf) {
        delete[] buffer; 
    }
    init_hstring();
    data_length = other.data_length;
    expand(data_length + 1);
    memcpy(buffer, other.buffer, data_length);
    buffer[data_length] = '\0';
    return *this;

}
hstring& hstring::operator=(const char* str)
{
    if (buffer != stack_buf) {
        delete[] buffer;
    }
    init_hstring();

    if (str == nullptr) {
        return *this;
    }

    //求数组长度
    unsigned len = 0;
    while (str[len] != '\0') {
        len++;
    }
    data_length = len;
    expand(data_length+1);
    memcpy(buffer, str, len);
    buffer[data_length] = '\0';
    return *this;
}
hstring& hstring::operator=(const int num)
{
    if (buffer != stack_buf) {
        delete[] buffer;
    }
    init_hstring();
    //int最大32位，数据长度超不过128位
    unsigned len = 0;
    char temp[64]; //暂存转成的字符串
    int_to_str(num, temp, len);
    data_length = len;
    expand(data_length + 1);//一般是运行不到，但是保险写一下
    memcpy(buffer, temp, data_length);
    buffer[data_length] = '\0';
    return *this;
}

//加法运算符重载
hstring& hstring::operator+(const hstring& other)
{
    hstring res;
    unsigned len = data_length + other.data_length;
    res.expand(len + 1);

    memcpy(res.buffer , this->buffer, data_length);
    memcpy(res.buffer + data_length, other.buffer, other.data_length);
    res.buffer[len] = '\0';
    res.data_length = len;
    return res;
}
hstring& hstring::operator+(const char* sub)
{
    hstring res;
    if (sub == nullptr) { //加的为空
        res = *this;
        return res;
    }

    //计算字符串长度
    unsigned len = 0;
    while (sub[len] != '\0') {
        len++;
    }

    //判断是否扩容
    res.expand(data_length + len + 1);
    res.data_length = data_length + len;

    //拷贝新数据进去
    memcpy(res.buffer , buffer, data_length);
    memcpy(res.buffer+data_length, sub ,len);
    res.buffer[res.data_length] = '\0';

    return res;
}

//减法运算符重载
hstring& hstring::operator-(const hstring& sub)
{
    int pos = find_sub(buffer, data_length, sub.buffer, sub.data_length);
    if (pos == -1) {
        std::cout << "[ERROR]子串\"" << sub.c_str() << "\"未找到!" << std::endl;
        return *this;
    }

    hstring res;
    unsigned len = data_length - sub.data_length;
    res.expand(len);
    memcpy(res.buffer, buffer, pos);
    memcpy(res.buffer + pos, buffer + pos + sub.data_length, data_length - pos - sub.data_length);
    res.buffer[len] = '\0';
    return res;

}
hstring& hstring::operator-(const char* sub)
{
    unsigned sub_len = 0;
    while (sub[sub_len] != '\0')
        sub_len++; //求长度

    int pos = find_sub(buffer, data_length, sub, sub_len);
    if (pos == -1)
    {
        std::cout << "[ERROR]子串\"" << sub << "\"未找到!" << std::endl;
        return *this;
    }

    hstring res;
    unsigned new_len = data_length - sub_len;
    memcpy(res.buffer, buffer, pos);
    memcpy(res.buffer + pos, buffer + pos + sub_len, data_length - pos - sub_len);
    res.buffer[new_len] = '\0';
    res.data_length = new_len;
    return res;
}

//修改替换操作
bool hstring::replace(unsigned index, const char* A, const char* B)
{
    int pos = find(A);
    if (pos == -1 || (unsigned) pos != index) {
        std::cout << "[ERROR]子串\"" << A << "\"未找到或当前修改子串不是第一个出现的子串!" << std::endl;
        return false;
    }
    
    unsigned l1 = 0;
    unsigned l2 = 0;
    while (A[l1] != '\0') {
        l1++;
    }
    while (B[l2] != '\0') {
        l2++;
    }

    unsigned new_len = data_length - l1 + l2;
    expand(new_len + 1);
    memcpy(buffer+pos+l2, buffer+pos+l1, data_length-pos-l1);//后缀拷贝
    memcpy(buffer + pos, B, l2);//插入新串
    buffer[new_len] = '\0';
    data_length = new_len;
    return true;
}

//重载左移运算符
std::ostream& operator<<(std::ostream& os, const hstring& str) {
    os << str.c_str();
    return os;
}


//将int转为str (char的数组)
void hstring::int_to_str(int num, char* result, unsigned& len)
{
    if (num == 0) {
        result[0] = '0';
        result[1] = '\0';
        len = 1;
        return;
    }

    len = 0;
    bool neg = (num < 0); //是否为负数
    unsigned temp;
    if (neg) {
        temp = ~(static_cast<unsigned>(num)) + 1U; //转成无符号数
    }
    else {
        temp = static_cast<unsigned>(num);
    }
    char list[64];
    do {
        list[len] = temp % 10;
        len++;
        temp /= 10;
    } while (temp != 0);
    
    if (neg) {
        len += 1;
        result[0] = '-';
        result[len] = '\0';
        for (int i = len; i > 1; i--) {
            result[i - 1] = '0' + list[len - i];
        }
    }
    else {
        result[len] = '\0';
        for (int i = len; i > 0; i--) {
            result[i - 1] = '0' + list[len - i];
        }
    }
    
    return;

}

//查找子串 KMP匹配
int hstring::find_sub(const char* list1, unsigned len1, const char* list2, unsigned len2)
{
    //边界：模式串为空，或者模式串比主串长
    if (len2 == 0 || len2 > len1)
        return -1;
    //构建前缀数组next
    int* next = new int[len2];
    next[0] = 0;   //第一个字符next固定0
    int i = 1;     //i：模式串后缀指针
    int j = 0;     //j：模式串前缀指针

    while (i < (int)len2)
    {
        if (list2[i] == list2[j])
        {
            j++;
            next[i] = j;
            i++;
        }
        else
        {
            if (j != 0)
            {
                j = next[j - 1];
            }
            else
            {
                next[i] = 0;
                i++;
            }
        }
    }

    //KMP匹配
    int p1 = 0; //主串指针
    int p2 = 0; //模式串指针
    while (p1 < (int)len1 && p2 < (int)len2)
    {
        if (list1[p1] == list2[p2])
        {
            p1++;
            p2++;
        }
        else
        {
            if (p2 != 0)
            {
                p2 = next[p2 - 1];
            }
            else
            {
                p1++;
            }
        }
    }

    int ret = -1;
    if (p2 >= (int)len2)
    {
        //匹配成功，返回起始下标
        ret = p1 - p2;
    }

    delete[] next; //释放next数组，防止内存泄漏
    return ret;

}

//返回子串索引
int hstring::find(const char* context)
{
    unsigned len = 0;
    while (context[len] != '\0') {
        len++;
    }
    int res = find_sub(buffer, data_length, context, len);
    if (res == -1) {
        std::cout << "[ERROR]子串\"" << context << "\"未找到!" << std::endl;
    }
    return res;
}






int main()
{   
    std::cout << "===== 1. 赋值运算符测试（字符串/数字） =====" << std::endl;
    hstring s1;
    s1 = "123456789";
    std::cout << "s1 = " << s1 << " 长度:" << s1.length() << std::endl;

    hstring s2;
    s2 = -12345;
    std::cout << "s2 = " << s2 << std::endl;

    hstring s3 = s1;
    std::cout << "s3拷贝s1: " << s3 << std::endl;

    std::cout << "\n===== 2. + 拼接运算符测试 =====" << std::endl;
    hstring res_add = s1 + "abc";
    std::cout << "s1 + \"abc\" = " << res_add << std::endl;

    hstring s4("xyz");
    hstring res_add2 = res_add + s4;
    std::cout << "res_add + s4 = " << res_add2 << std::endl;

    std::cout << "\n===== 3. - 删除子串运算符测试 =====" << std::endl;
    hstring s_del("123456789");
    hstring res_del = s_del - "456";
    std::cout << "123456789 - \"456\" = " << res_del << std::endl;
    hstring err_del = s_del - "999"; // 不存在子串，输出错误

    std::cout << "\n===== 4. find 查找测试 =====" << std::endl;
    hstring s_find("123456");
    int pos = s_find.find("34");
    std::cout << "find(\"34\") 返回下标:" << pos << std::endl;
    s_find.find("99");

    std::cout << "\n===== 5. replace 替换测试 =====" << std::endl;
    hstring s_rep("123456789");
    bool ok = s_rep.replace(2, "34", "abc");
    if (ok) {
        std::cout << "替换后: " << s_rep << std::endl;
    }
    ok = s_rep.replace(3, "34", "abc");

    std::cout << "\n===== 6. 边界测试：空串、单字符 =====" << std::endl;
    hstring empty("");
    std::cout << "空串长度:" << empty.length() << std::endl;
    hstring single("A");
    hstring single_del = single - "A";
    std::cout << "单字符删除后: [" << single_del << "]" << std::endl;

    return 0;
}

