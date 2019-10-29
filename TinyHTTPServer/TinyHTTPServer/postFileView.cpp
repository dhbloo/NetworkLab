#include "postFileView.h"

#include "request.h"
#include "requestExcept.h"
#include "response.h"
#include "util.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

PostFileView::PostFileView(std::string path) : directory(path)
{
    if (directory.empty() || directory.back() != '/')
        directory.append("/");
}

void PostFileView::handle(Request &request, Response &response)
{
    std::string urlFilePath;

    auto dirKV = request.urlParams.find("filepath");
    if (dirKV != request.urlParams.end())
        urlFilePath = dirKV->second;

    fs::path filePath = fs::u8path(directory + urlFilePath);
    bool     isDir    = fs::is_directory(filePath);

    if (request.method == Request::DEL) {
        if (!fs::exists(filePath))
            throw Abort(404, "File not found: " + filePath.string());

        if (filePath == "/")
            filePath.clear();

        fs::remove_all(filePath);
    }
    else {
        // 对于目录, 创建相应的空目录
        if (isDir) {
            fs::create_directories(filePath);
        }
        // 对于文件, 创建并写入文件内容
        else {
            fs::path dirpath = filePath;
            fs::create_directories(dirpath.remove_filename());
            std::ofstream fileStream(filePath, std::ios_base::binary);
            fileStream << request.body;
        }

        response.statusCode = 201;
    }
}
