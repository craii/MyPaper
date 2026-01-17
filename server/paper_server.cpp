#include "paper_server.h"

void WallpaperRequestHandler::handleRequest(HTTPServerRequest& request,
                                            HTTPServerResponse& response) 
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Object json;
    std::ostream& out = response.send();

    try
    {
        if (request.getMethod() != HTTPRequest::HTTP_POST)
        {
            response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
            json.set("code", 400);
            json.set("msg", "POST required");
            json.set("data_full", Array::Ptr(new Array));
            json.set("data_half", Array::Ptr(new Array));
            json.stringify(out);
            return;
        }

        // 解析 POST 参数
        HTMLForm form(request, request.stream());
        std::string token = form.get("token", "");

        // test token
        bool token_exist = this->TokenController->exists(token);
        if (!token_exist)
        {
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            json.set("code", 401);
            json.set("msg", "invalid token");
            json.set("data_full", Array::Ptr(new Array));
            json.set("data_half", Array::Ptr(new Array));
            json.stringify(out);
            return;
        }

        // 构造 vector<string>
        std::vector<std::string> vec, vec_hf;

        // vector<string> → JSON Array
        // std::string path = this->Controller->get_full_pic_path();
        // std::cout << "PRINT FULL PATH IN WallpaperRequestHandler: "<< path << std::endl;

        Controller->scan_folder(Controller->get_full_pic_path());
        Controller->scan_folder(Controller->get_half_pic_path());
        vector<string> full_pics = Controller->get_vector_f();
        vec.clear();
        copy(full_pics.begin(), full_pics.end(), std::back_inserter(vec));

        vector<string> half_pics = Controller->get_vector_hf();
        vec_hf.clear();
        copy(half_pics.begin(), half_pics.end(), std::back_inserter(vec_hf));



        Array::Ptr arr = new Array;
        for (const auto& s : vec)
        {
            arr->add(s);
        }

        Array::Ptr arr_hf = new Array;
        for (const auto& s : vec_hf)
        {
            arr_hf->add(s);
        }

        response.setStatus(HTTPResponse::HTTP_OK);
        json.set("code", 200);
        json.set("msg", "ok");
        json.set("data_full", arr);
        json.set("data_half", arr_hf);

        json.stringify(out);
    }
    catch (const std::exception& e)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        json.set("code", 500);
        json.set("msg", e.what());
        json.set("data_full", Array::Ptr(new Array));
        json.set("data_half", Array::Ptr(new Array));
        json.stringify(out);
    }

}

// 在 WallpaperRequestHandler::handleRequest 之后添加以下实现

/**
 * 处理 /refresh 请求
 * 使用 TEST_TOKENS 时,扫描文件夹并压缩缺失的 half_ 版本图片
 */
void RefreshRequestHandler::handleRequest(HTTPServerRequest& request,
                                          HTTPServerResponse& response) 
{
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Object json;
    std::ostream& out = response.send();

    try
    {
        if (request.getMethod() != HTTPRequest::HTTP_GET)
        {
            response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
            json.set("code", 400);
            json.set("msg", "GET required");
            json.set("processed", 0);
            json.set("failed", 0);
            json.set("details", Array::Ptr(new Array));
            json.stringify(out);
            return;
        }

        // 从 URL 查询参数中获取 token
        Poco::URI uri(request.getURI());
        Poco::URI::QueryParameters params = uri.getQueryParameters();
        
        std::string token;
        for (const auto& param : params)
        {
            if (param.first == "token")
            {
                token = param.second;
                break;
            }
        }

        // 检查是否为测试 token
        if (token != "TEST_TOKENS")
        {
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            json.set("code", 401);
            json.set("msg", "invalid token, requires TEST_TOKENS");
            json.set("processed", 0);
            json.set("failed", 0);
            json.set("details", Array::Ptr(new Array));
            json.stringify(out);
            return;
        }

        // 调用 walk_folder 获取需要压缩的文件列表
        std::vector<std::string> files_to_compress = Controller->walk_folder();
        
        if (files_to_compress.empty())
        {
            response.setStatus(HTTPResponse::HTTP_OK);
            json.set("code", 200);
            json.set("msg", "no files need compression");
            json.set("processed", 0);
            json.set("failed", 0);
            json.set("details", Array::Ptr(new Array));
            json.stringify(out);
            return;
        }

        // 压缩文件
        int processed_count = 0;
        int failed_count = 0;
        Array::Ptr details = new Array;

        for (const auto& source_path : files_to_compress)
        {
            try
            {
                namespace fs = std::filesystem;
                
                // 构建目标文件路径
                std::string filename = fs::path(source_path).filename().string();
                std::string stem = fs::path(source_path).stem().string();
                std::string ext = fs::path(source_path).extension().string();
                
                std::string dest_filename = "half_" + stem + ext;
                std::string dest_path = Controller->get_half_pic_path() + "/" + dest_filename;

                // 调用压缩函数
                int64_t result_size = Controller->compress_picture(
                    source_path, 
                    dest_path,
                    ImageController::Quality::BROKEN
                );

                Object file_result;
                file_result.set("source", filename);
                file_result.set("destination", dest_filename);
                
                if (result_size > 0)
                {
                    file_result.set("status", "success");
                    file_result.set("size", static_cast<int>(result_size));
                    processed_count++;
                }
                else
                {
                    file_result.set("status", "failed");
                    file_result.set("error", "compression returned error");
                    failed_count++;
                }
                
                details->add(file_result);
            }
            catch (const std::exception& e)
            {
                Object file_result;
                file_result.set("source", fs::path(source_path).filename().string());
                file_result.set("status", "failed");
                file_result.set("error", e.what());
                details->add(file_result);
                failed_count++;
            }
        }

        response.setStatus(HTTPResponse::HTTP_OK);
        json.set("code", 200);
        json.set("msg", "refresh completed");
        json.set("processed", processed_count);
        json.set("failed", failed_count);
        json.set("total", static_cast<int>(files_to_compress.size()));
        json.set("details", details);

        json.stringify(out);
    }
    catch (const std::exception& e)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        json.set("code", 500);
        json.set("msg", e.what());
        json.set("processed", 0);
        json.set("failed", 0);
        json.set("details", Array::Ptr(new Array));
        json.stringify(out);
    }
}

// 新增：图片文件处理实现
std::string ImageFileHandler::getMimeType(const std::string& path)
{
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".bmp") return "image/bmp";
    if (ext == ".webp") return "image/webp";
    if (ext == ".svg") return "image/svg+xml";
    
    return "application/octet-stream";
}

void ImageFileHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    try
    {
        // 检查 HTTP 方法，只允许 GET 和 HEAD
        std::string method = request.getMethod();
        if (method != HTTPRequest::HTTP_GET && method != HTTPRequest::HTTP_HEAD)
        {
            response.setStatus(HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
            response.set("Allow", "GET, HEAD");
            response.setContentType("text/plain");
            std::ostream& out = response.send();
            out << "Method not allowed. Use GET or HEAD.";
            return;
        }

        // Token 验证
        std::string token;
        
        // 从 URL 查询参数中获取 token (例如: /images/xxx.jpg?token=my_secret_token)
        Poco::URI uri(request.getURI());
        Poco::URI::QueryParameters params = uri.getQueryParameters();
        
        for (const auto& param : params)
        {
            if (param.first == "token")
            {
                token = param.second;
                break;
            }
        }
        
        // 验证 token
        bool token_exist = this->TokenController->exists(token);
        if (!token_exist)
        {
            response.setStatus(HTTPResponse::HTTP_UNAUTHORIZED);
            response.setContentType("text/plain");
            std::ostream& out = response.send();
            out << "Unauthorized: invalid token";
            return;
        }

        // 检查文件是否存在
        if (!fs::exists(m_filePath) || !fs::is_regular_file(m_filePath))
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            response.setContentType("text/plain");
            std::ostream& out = response.send();
            out << "File not found";
            return;
        }

        // 获取文件大小
        auto fileSize = fs::file_size(m_filePath);
        
        // 打开文件
        std::ifstream file(m_filePath, std::ios::binary);
        if (!file)
        {
            response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.setContentType("text/plain");
            std::ostream& out = response.send();
            out << "Failed to open file";
            return;
        }

        // 设置响应头
        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType(getMimeType(m_filePath));
        response.setContentLength(fileSize);
        
        // 添加 Content-Disposition 头，支持浏览器下载
        std::string filename = fs::path(m_filePath).filename().string();
        response.set("Content-Disposition", "inline; filename=\"" + filename + "\"");

        // 发送文件内容（如果是 HEAD 请求，则不发送内容）
        std::ostream& out = response.send();
        
        if (method == HTTPRequest::HTTP_GET)
        {
            char buffer[8192];
            while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
            {
                out.write(buffer, file.gcount());
            }
        }
        
        file.close();
    }
    catch (const std::exception& e)
    {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.setContentType("text/plain");
        std::ostream& out = response.send();
        out << "Error: " << e.what();
    }
}


HTTPRequestHandler* HelloRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request) 
{
    std::string uri = request.getURI();
    
    // 调试输出
    std::cout << "Received URI: " << uri << std::endl;
    
    // 处理 /wallpaper API
    if (uri == "/wallpaper")
    {
        return new WallpaperRequestHandler(this->Controller, this->TokenController);
    }

    // 新增: 处理 /refresh API
    // 支持 /refresh 或 /refresh?token=xxx
    if (uri == "/refresh" || uri.find("/refresh?") == 0)
    {
        return new RefreshRequestHandler(this->Controller, this->TokenController);
    }
    
    // 处理图片文件请求 /images/...
    if (uri.find("/images/") == 0)
    {
        // 使用 Poco::URI 解析，分离路径和查询参数
        Poco::URI parsedUri(uri);
        std::string path = parsedUri.getPath();  // 只获取路径部分，不包含查询参数
        
        // 去掉开头的 '/'，构造相对路径
        std::string filePath = path.substr(1);  // 移除开头的 '/'
        
        std::cout << "File path: " << filePath << std::endl;
        
        // 安全检查：防止路径遍历攻击
        if (filePath.find("..") != std::string::npos)
        {
            std::cout << "Path traversal detected!" << std::endl;
            return nullptr;
        }
        
        // 检查文件是否存在（仅用于调试）
        if (!fs::exists(filePath))
        {
            std::cout << "WARNING: File does not exist: " << filePath << std::endl;
            std::cout << "Current working directory: " << fs::current_path() << std::endl;
        }
        else
        {
            std::cout << "File found: " << filePath << std::endl;
        }
        
        return new ImageFileHandler(filePath, this->TokenController);
    }
    
    // 对于未知路径，返回 nullptr（会返回 501）
    std::cout << "Unknown URI, returning nullptr" << std::endl;
    return nullptr;
}


int PocoServerApp::main(const std::vector<std::string>& args) 
{
    
    // UInt16 port = 9090; 
    UInt16 port = this->server_port;

    ServerSocket socket(port);
    HTTPServer server(
        new HelloRequestHandlerFactory(this->Controller, this->TokenController),
        socket,
        new HTTPServerParams
    );

    server.start();
    logger().information("Poco HTTP Server started on port %hu", port);

    waitForTerminationRequest();
    server.stop();

    logger().information("Server stopped");
    return Application::EXIT_OK;
}