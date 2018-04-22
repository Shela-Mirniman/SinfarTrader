#pragma once
#include <string>
#include <functional>

inline std::string ColorText(int red,int green,int blue,std::string text)
{
    return std::string("<c")+char(red)+char(green)+char(blue)+std::string(">")+text+std::string("</c>");                                                                     
}

static auto ColorRed=std::bind(ColorText,255,0,0,std::placeholders::_1);
static auto ColorGreen=std::bind(ColorText,0,255,0,std::placeholders::_1);
static auto ColorBlue=std::bind(ColorText,0,0,255,std::placeholders::_1);
static auto ColorPurple=std::bind(ColorText,128,0,128,std::placeholders::_1);
