#pragma once
#include <string>
#include <functional>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp> 
#include <boost/iostreams/code_converter.hpp>
#include <boost/locale.hpp>

/*inline std::string ToPrintableASCII(char value)
{
    std::string res;
    if(value<32)
    {
        res+=char(32);
    }
    else if(value>126&&value<=254)
    {
	res+=(value+33);
    }
    else if(value>254)
    {
        res+=char(254);
    }
    else
    {
        res+=value;
    }
    return res; 
}

inline std::string ColorText(int red,int green,int blue,std::string text)
{
    return std::string("<c")+ToPrintableASCII(red)+ToPrintableASCII(green)+ToPrintableASCII(blue)+std::string(">")+text+std::string("</c>")+ToPrintableASCII(red)+ToPrintableASCII(green)+ToPrintableASCII(blue);
}
*/

inline std::string ColorRed(std::string text)
{
    return std::string("<c´  >")+text+std::string("</c>");
}
/*
static auto ColorGreen=std::bind(ColorText,0,255,0,std::placeholders::_1);
static auto ColorBlue=std::bind(ColorText,0,0,255,std::placeholders::_1);
static auto ColorPurple=std::bind(ColorText,128,0,128,std::placeholders::_1);
*/
