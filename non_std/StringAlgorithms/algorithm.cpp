#include "algorithm.hpp"

std::string ltrim(const std::string& in)
{
    auto pos = in.find_first_not_of(' ');
    if (pos != std::string::npos)
    {
        return in.substr(in.find_first_not_of(' '));
    }
    return in;
}

std::string rtrim(const std::string& in)
{
    auto count = std::find_if(in.rbegin(), in.rend(), [](char c) { return c != ' '; }) - in.rbegin();
    return in.substr(0, in.size() - count);
}

std::string trim(const std::string& in)
{
    return rtrim(ltrim(in));
}

std::vector<std::string> splitAndTrim(const std::string& in, char c)
{
    std::vector<std::string> out;
    for (decltype(in.size()) i = 0; i < in.size();)
    {
        auto pos = in.find(c, i);
        out.emplace_back(trim(in.substr(i, pos-i)));
        if (pos == std::string::npos)
            break;
        i = pos + 1;
    }
    return out;
}

std::vector<std::string> splitAndTrimByStr(const std::string& in, std::string delimeter)
{
    std::vector<std::string> out;
    for (decltype(in.size()) i = 0; i < in.size();)
    {
        auto pos = in.find(delimeter, i);
        out.emplace_back(trim(in.substr(i, pos-i)));
        if (pos == std::string::npos)
            break;
        i = pos + delimeter.size();
    }
    return out;
}

std::vector<std::string> splitNumbersAndLetters(const std::string& in)
{
    std::vector<std::string> out;
    for (decltype(in.size()) i = 0; i < in.size();)
    {
        std::string number;
        while (std::isdigit(in[i]))
        {
            number.push_back(in[i++]);
        }
        if (number.size())
        {
            out.push_back(number);
        }

        std::string letters;
        while (std::isalpha(in[i]))
        {
            letters.push_back(in[i++]);
        }
        if (letters.size())
        {
            out.push_back(letters);
        }
    }
    return out;
}