#include "libvdf.hpp"

int main()
{
    std::string input("\"a\" \"abc\"");
    libvdf::VDFReader reader;
    bool bRet = reader.Parser(input);
    if (bRet)
    {
        libvdf::Node root = reader.Root();
        root["a"].value = "def";
        libvdf::VDFWriter writer;
        std::string output;
        bRet = writer.Write(root, output);
    }

    return 0;
}
