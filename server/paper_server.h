#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTMLForm.h>

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Application.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>

#include <Poco/URI.h>

#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "image_controller.h"
#include "token_store.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace Poco::JSON;

namespace fs = std::filesystem;


class HelloRequestHandlerFactory : public HTTPRequestHandlerFactory
{
    public:
        HelloRequestHandlerFactory(ImageController* controller, 
                                   TokenStore* token_controller): Controller(controller), TokenController(token_controller){}

        HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override;
    private:
        ImageController* Controller;
        TokenStore* TokenController;
};

class WallpaperRequestHandler : public HTTPRequestHandler
{
    public:
        WallpaperRequestHandler(ImageController* controller, 
                                TokenStore* token_controller): Controller(controller), TokenController(token_controller){}
        void handleRequest(HTTPServerRequest& request,HTTPServerResponse& response) override;
    private:
        ImageController* Controller;
        TokenStore* TokenController;
    
};


// 新增: 处理 /refresh 请求
class RefreshRequestHandler : public HTTPRequestHandler
{
    public:
        RefreshRequestHandler(ImageController* controller, 
                             TokenStore* token_controller): Controller(controller), TokenController(token_controller){}
        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
    private:
        ImageController* Controller;
        TokenStore* TokenController;
};


//处理图片
class ImageFileHandler : public HTTPRequestHandler
{
    public:
        ImageFileHandler(const std::string& filePath, TokenStore* token_controller): m_filePath(filePath), TokenController(token_controller){}
        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
    private:
        std::string m_filePath;
        std::string getMimeType(const std::string& path);
        TokenStore* TokenController;
};


class PocoServerApp : public ServerApplication
{
    public:
        PocoServerApp(UInt16 port, 
                      ImageController* controller,
                      TokenStore* token_controller): server_port(port), Controller(controller), TokenController(token_controller){}
        ~PocoServerApp(){
            // delete Controller;
            // delete TokenController;
            // 没看文档卧槽
        }

    private:
        UInt16 server_port = 8080;
        ImageController* Controller;
        TokenStore* TokenController;
    
    protected:
        int main(const std::vector<std::string>& args) override;
};