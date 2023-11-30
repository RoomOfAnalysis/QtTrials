#include <filesystem>

#include <quazip/JlCompress.h>
#include <QDebug>

// 1) dir to compress; 2) compressed file path
int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;

    auto dir = fs::u8path(argv[1]);
    if (!fs::exists(dir))
    {
        qDebug() << "dir not exist";
        std::abort();
    }

    if (auto is_compressed = JlCompress::compressDir(argv[2], argv[1], true); !is_compressed)
    {
        qDebug() << "failed to compress:" << dir;
        std::abort();
    }

    auto extracted_dir = dir.parent_path() / "_extract/";
    if (auto l = JlCompress::extractDir(argv[2], extracted_dir.u8string().c_str()); l.isEmpty())
        qDebug() << "failed to extract to:" << extracted_dir;
    else
        for (auto const& s : l)
            qDebug() << s;

    return 0;
}