#include "../includes/utils.h"
namespace Utils
{
    std::string read_word(const std::string &content, int &pos)
    {
        std::string ret;
        while (pos < content.size() && (content[pos] == '_' || isalnum(content[pos])))
            ret += content[pos++];
        return ret;
    }
    
    Slicer::Slicer(const std::string &file_content)
    {
        int pos = 0;
        bool fpush_new = true;
        while (pos < file_content.size())
        {
            if (file_content[pos] == '$')
            {
                auto key = read_word(file_content, ++pos);
                mut_idxs.insert({key, slices.size()});
                slices.push_back({});
                slices.back().is_mutable = true;
                fpush_new = true;
            }
            else
            {
                if (fpush_new)
                    fpush_new = false, slices.push_back({});
                slices.back().content += file_content[pos++];
            }
        }
    }

    std::string &Slicer::get_mut(const std::string &idx_name)
    {
        if (!mut_idxs.count(idx_name))
            throw std::runtime_error("unknown mutable segmentation " + idx_name);
        return slices[mut_idxs[idx_name]].content;
    }

    std::string Slicer::merge() const
    {
        std::string ret;
        for (auto item : slices)
            ret += item.content;
        return ret;
    }

}