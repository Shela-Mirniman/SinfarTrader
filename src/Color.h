#pragma once
#include <string>
#include <functional>

inline int ToPrintableASCII(int value)
{
    if(value<32)
    {
	return 32;
    }
    else if(value>254)
    {
	return 254;
    }
    return value;
}
inline std::string ColorText(int red,int green,int blue,std::string text)
{
    return std::string("<c")+char(ToPrintableASCII(red))+char(ToPrintableASCII(green))+char(ToPrintableASCII(blue))+std::string(">")+text+std::string("</c>");                                                                     
}

static auto ColorRed=std::bind(ColorText,231,32,32,std::placeholders::_1);
static auto ColorGreen=std::bind(ColorText,0,255,0,std::placeholders::_1);
static auto ColorBlue=std::bind(ColorText,0,0,255,std::placeholders::_1);
static auto ColorPurple=std::bind(ColorText,128,0,128,std::placeholders::_1);
