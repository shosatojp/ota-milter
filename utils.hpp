#include <string>
#include <vector>
#include <sstream>

namespace ota::utils
{
    std::string str_join(const std::vector<std::string> &v, const std::string &delim = "")
    {
        std::ostringstream ss;
        for (size_t i = 0, len = v.size(); i < len; i++)
        {
            if (i != 0)
                ss << delim;
            ss << v[i];
        }
        return ss.str();
    }

    std::vector<std::string> str_split(const std::string &str, const std::string &delim)
    {
        std::vector<std::string> v;
        size_t start = 0, end = 0;
        while ((end = str.find(delim, start)) != std::string::npos)
        {
            v.push_back(str.substr(start, end - start));
            start = end + delim.size();
        }
        v.push_back(str.substr(start));
        return v;
    }

    std::string str_trim(const std::string &str, const std::string &white = " \n\t\r")
    {
        const size_t start = str.find_first_not_of(white),
                     end = str.find_last_not_of(white);
        return start < end ? str.substr(start, end - start + 1) : "";
    }

}
