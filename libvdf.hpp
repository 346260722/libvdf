#ifndef __LIB_VDF__
#define __LIB_VDF__

#include <string>
#include <vector>
#include <fstream>

namespace libvdf
{
    enum class NodeType
    {
        UNKNOWN,
        VALUE,
        ARRAY
    };

    class Node
    {
    public:
        std::string name;
        std::string value;
        NodeType    type = NodeType::UNKNOWN;

        void Add(const Node& node)
        {
            vc_.push_back(node);
        }

        Node& Get(size_t i)
        {
            return vc_.at(i);
        }

        Node& Get(const std::string& name, Node&& default_node)
        {
            for (auto& iter : vc_)
            {
                if (iter.name == name)
                    return iter;
            }

            return default_node;
        }

        bool Exists(const std::string& name)
        {
            for (auto& iter : vc_)
            {
                if (iter.name == name)
                    return true;
            }

            return false;
        }

        size_t Size() const
        {
            return vc_.size();
        }

        Node& operator[](const std::string& name)
        {
            Node* ptr = nullptr;
            for (auto& iter : vc_)
            {
                if (iter.name == name)
                {
                    ptr = &iter;
                    break;
                }
            }

            return *ptr;
        }

    private:
        std::vector<Node> vc_;
    };

    class VDFReader
    {
    public:
        bool ParserFromFile(const std::string& file_path)
        {
            std::ifstream ifs(file_path);
            if (!ifs.is_open())
                return false;

            std::string buffer((std::istreambuf_iterator<char>(ifs)),
                std::istreambuf_iterator<char>());
            ifs.close();

            return Parser(buffer);
        }

        bool Parser(const std::string& input)
        {
            std::string buffer(input);
            root_.type = NodeType::ARRAY;
            return Parser_(buffer, root_);
        }

        Node& Root() { return root_; }

    private:
        bool Parser_(std::string& input, Node& root)
        {
            std::string::size_type pos_start, pos_end;
            while (true)
            {
                pos_start = input.find_first_of("\"}");
                if (pos_start == std::string::npos)
                    return true;

                if (input[pos_start] == '}')
                {
                    input = input.substr(pos_start + 1);
                    return true;
                }

                pos_start += 1;
                pos_end = input.find("\"", pos_start);
                if (pos_end == std::string::npos)
                    return false;

                Node temp;
                temp.name = input.substr(pos_start, pos_end - pos_start);
                input = input.substr(pos_end + 1);

                pos_start = input.find_first_of("\"{");
                if (pos_start == std::string::npos)
                    return false;

                if (input[pos_start] == '{')
                {
                    temp.type = NodeType::ARRAY;
                    if (!Parser_(input, temp))
                        return false;
                }
                else
                {
                    pos_start += 1;
                    pos_end = input.find("\"", pos_start);
                    if (pos_end == std::string::npos)
                        return false;

                    temp.type = NodeType::VALUE;
                    temp.value = input.substr(pos_start, pos_end - pos_start);
                    input = input.substr(pos_end + 1);
                }

                root.Add(std::move(temp));
            }

            return true;
        }

        Node    root_;
    };

    class VDFWriter
    {
    public:
        bool WriteToFile(Node& root, const std::string& file_path)
        {
            std::ofstream ofs(file_path);
            if (!ofs.is_open())
                return false;

            std::string buffer;
            if (!Write(root, buffer))
                return false;

            ofs.write(buffer.c_str(), buffer.size());
            ofs.close();

            return true;
        }

        bool Write(Node& root, std::string& output)
        {
            return Write_(root, 0, output);
        }

    private:
        bool Write_(Node& root, const int level, std::string& output)
        {
            if (root.name.empty())
            {
                if (root.Size() > 0)
                    return Write_(root.Get(0), level, output);
                return true;
            }

            for (int i = 0; i < level; ++i)
                output += "\t";

            output += "\"";
            output += root.name;
            output += "\"";
            if (root.type == NodeType::ARRAY)
            {
                output += "\n";
                for (int i = 0; i < level; ++i)
                    output += "\t";
                output += "{";
                output += "\n";

                for (size_t i = 0; i < root.Size(); ++i)
                {
                    if (!Write_(root.Get(i), level + 1, output))
                        return false;
                }

                for (int i = 0; i < level; ++i)
                    output += "\t";
                output += "}\n";
            }
            else
            {
                output += "\t\t";
                output += "\"";
                output += root.value;
                output += "\"";
                output += "\n";
            }

            return true;
        }
    };
}

#endif // __LIB_VDF__
