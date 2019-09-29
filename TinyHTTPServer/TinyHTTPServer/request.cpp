#include "request.h"
#include <cassert>
#include <vector>
#include <stdexcept>

Request Request::parse(std::stringstream& ss) {
    Request request;

    ss >> request.methodStr >> request.url >> request.version;
    if (request.methodStr == "GET") {
        request.method = Request::GET;
    }
    else if (request.methodStr == "HEAD") {
        request.method = Request::HEAD;
    }
    else {
        std::vector<std::string> methods = {
            "POST",
            "PUT",
            "DELETE",
            "OPTIONS",
            "TRACE",
            "CONNECT"
        };

        if (std::find(methods.begin(), methods.end(), request.methodStr) != methods.end()) {
            request.method = Request::UNSUPPORTED;
            return request;
        }
        else {
            throw std::runtime_error("Unable to parse request");
        }
    }

    std::string buf = ss.str();
    size_t lineStartPos;
    size_t lineEndPos = buf.find("\r\n");
    do {
        lineStartPos = lineEndPos + 2;
        lineEndPos = buf.find("\r\n", lineStartPos);
        if (lineEndPos > lineStartPos) {
            std::string_view line(buf.data() + lineStartPos, lineEndPos - lineStartPos);
            size_t delimPos = line.find_first_of(':');
            assert(delimPos != std::string::npos);

            request.headers[std::string(line.substr(0, delimPos))] = line.substr(delimPos + 2);
        }
    } while (lineEndPos > lineStartPos);

    std::string_view body(buf);
    request.body = buf.substr(lineEndPos + 2);

    return request;
}
