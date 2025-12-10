#include "languagemanager.h"
#include <QDebug>

LanguageManager& LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager(QObject* parent)
    : QObject(parent)
    , m_currentLanguage(Language::Chinese)
    , m_translator(new QTranslator(this))
{
    initTranslations();
}

LanguageManager::~LanguageManager()
{
    if (m_translator) {
        qApp->removeTranslator(m_translator);
    }
}

void LanguageManager::initTranslations()
{
    // 初始化翻译映射表
    m_translations = {
        // 主窗口
        {"main_title", {{Language::Chinese, "航班票务管理系统 - FlyOver"}, 
                        {Language::English, "Flight Booking System - FlyOver"}}},
        {"home", {{Language::Chinese, "🏠 主页"}, 
                  {Language::English, "🏠 Home"}}},
        {"flight_query", {{Language::Chinese, "🔍 航班查询"}, 
                          {Language::English, "🔍 Flight Search"}}},
        {"my_orders", {{Language::Chinese, "📋 我的订单"}, 
                       {Language::English, "📋 My Orders"}}},
        {"travel_moments", {{Language::Chinese, "✈️ 旅行动态"}, 
                            {Language::English, "✈️ Travel Moments"}}},
        {"my_profile", {{Language::Chinese, "👤 我的信息"}, 
                        {Language::English, "👤 My Profile"}}},
        {"ai_assistant", {{Language::Chinese, "🤖 AI助手"}, 
                          {Language::English, "🤖 AI Assistant"}}},
        {"flight_management", {{Language::Chinese, "✈️ 航班管理"}, 
                               {Language::English, "✈️ Flight Management"}}},
        {"logout", {{Language::Chinese, "🚪 退出登录"}, 
                    {Language::English, "🚪 Logout"}}},
        
        // 航班信息
        {"flight_number", {{Language::Chinese, "航班号"}, 
                           {Language::English, "Flight No."}}},
        {"departure", {{Language::Chinese, "出发地"}, 
                       {Language::English, "Departure"}}},
        {"destination", {{Language::Chinese, "目的地"}, 
                         {Language::English, "Destination"}}},
        {"depart_time", {{Language::Chinese, "出发时间"}, 
                         {Language::English, "Depart Time"}}},
        {"arrive_time", {{Language::Chinese, "到达时间"}, 
                         {Language::English, "Arrive Time"}}},
        {"price", {{Language::Chinese, "价格"}, 
                   {Language::English, "Price"}}},
        {"available_seats", {{Language::Chinese, "余票"}, 
                             {Language::English, "Available"}}},
        
        // 操作按钮
        {"search", {{Language::Chinese, "🔍 搜索"}, 
                    {Language::English, "🔍 Search"}}},
        {"book", {{Language::Chinese, "预订"}, 
                  {Language::English, "Book"}}},
        {"cancel", {{Language::Chinese, "取消"}, 
                    {Language::English, "Cancel"}}},
        {"confirm", {{Language::Chinese, "确认"}, 
                     {Language::English, "Confirm"}}},
        {"refresh", {{Language::Chinese, "🔄 刷新"}, 
                     {Language::English, "🔄 Refresh"}}},
        {"add", {{Language::Chinese, "➕ 添加"}, 
                 {Language::English, "➕ Add"}}},
        {"delete", {{Language::Chinese, "🗑️ 删除"}, 
                    {Language::English, "🗑️ Delete"}}},
        {"edit", {{Language::Chinese, "✏️ 编辑"}, 
                  {Language::English, "✏️ Edit"}}},
        {"save", {{Language::Chinese, "💾 保存"}, 
                  {Language::English, "💾 Save"}}},
        
        // 会员系统
        {"points", {{Language::Chinese, "积分"}, 
                    {Language::English, "Points"}}},
        {"balance", {{Language::Chinese, "飞机币余额"}, 
                     {Language::English, "Coin Balance"}}},
        {"mileage", {{Language::Chinese, "飞行里程"}, 
                     {Language::English, "Mileage"}}},
        {"member_level", {{Language::Chinese, "会员等级"}, 
                          {Language::English, "Member Level"}}},
        
        // 通知消息
        {"success", {{Language::Chinese, "成功"}, 
                     {Language::English, "Success"}}},
        {"error", {{Language::Chinese, "错误"}, 
                   {Language::English, "Error"}}},
        {"warning", {{Language::Chinese, "警告"}, 
                     {Language::English, "Warning"}}},
        {"info", {{Language::Chinese, "提示"}, 
                  {Language::English, "Info"}}},
        
        // 登机牌
        {"boarding_pass", {{Language::Chinese, "电子登机牌"}, 
                           {Language::English, "Boarding Pass"}}},
        {"passenger_name", {{Language::Chinese, "乘客姓名"}, 
                            {Language::English, "Passenger Name"}}},
        {"seat_number", {{Language::Chinese, "座位号"}, 
                         {Language::English, "Seat"}}},
        {"gate", {{Language::Chinese, "登机口"}, 
                  {Language::English, "Gate"}}},
        {"terminal", {{Language::Chinese, "航站楼"}, 
                      {Language::English, "Terminal"}}},
        {"print", {{Language::Chinese, "打印登机牌"}, 
                   {Language::English, "Print Boarding Pass"}}},
    };
}

void LanguageManager::switchLanguage(Language lang)
{
    if (m_currentLanguage == lang) {
        return; // 已经是当前语言
    }
    
    m_currentLanguage = lang;
    
    qDebug() << "切换语言到：" << (lang == Language::Chinese ? "中文" : "English");
    
    emit languageChanged(lang);
}

QString LanguageManager::tr(const QString& key) const
{
    if (m_translations.contains(key)) {
        return m_translations[key].value(m_currentLanguage, key);
    }
    return key; // 如果找不到翻译，返回原key
}
