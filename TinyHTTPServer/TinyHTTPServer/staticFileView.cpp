#include "staticFileView.h"
#include "request.h"
#include "response.h"
#include "requestExcept.h"
#include "util.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>


const std::unordered_map<std::string, std::string> MimeType = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"css", "text/css"},
    {"js", "text/javascript"},
    {"txt", "text/plain"},
    {"xml", "text/xml"},
    {"markdown", "text/markdown"},

    {"gif", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"bmp", "image/bmp"},
    {"ico", "image/x-icon"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},

    {"json", "application/json"},
    {"wasm", "application/wasm"},
    {"pdf", "application/pdf"}
};


namespace fs = std::filesystem;

std::string StaticFileView::DefaultExt = ".html";

StaticFileView::StaticFileView(std::string path) : directory(path) {
    if (directory.back() != '/')
        directory.append("/");
}

void StaticFileView::handle(Request& request, Response& response) {
    std::string urlFilePath;

    auto dirKV = request.urlParams.find("filepath");
    if (dirKV != request.urlParams.end())
        urlFilePath = dirKV->second;

    urlFilePath = UrlDecode(urlFilePath);

    fs::path filePath = fs::u8path(directory + urlFilePath);
    bool isDir = fs::is_directory(filePath);
    bool hasExt = filePath.has_extension();

    // 对于目录查找目录下的index.(ext)
    if (isDir)
        filePath /= "index" + DefaultExt;
    // 对于没带扩展名的文件, 自动添加默认扩展名
    else if (!hasExt)
        filePath.replace_extension(DefaultExt);

    if (!fs::exists(filePath))
        throw Abort(404, "File not found: " + filePath.string());

    std::ifstream fileStream(filePath, std::ios_base::binary);
    response.body.assign(
        std::istreambuf_iterator<char>(fileStream),
        std::istreambuf_iterator<char>()
    );
    
    auto contentTypeIt = MimeType.find(filePath.extension().string().substr(1));
    if (contentTypeIt != MimeType.end())
        response.headers["Content-Type"] = contentTypeIt->second;
    else
        response.headers["Content-Type"] = "application/octet-stream;";
}

