#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include "image_controller.h"
#include "paper_server.h"
#include "token_store.h"


namespace fs = std::filesystem;


int main(int argc, char** argv)
{
    auto Controller = std::make_unique<ImageController>();

    Poco::Data::SQLite::Connector::registerConnector();
    auto TokenController = std::make_unique<TokenStore>("./tokends.db");
    TokenController->init();

    //初始化运行参数
    int port = 8080;
    std::string full_folder_path;
    std::string half_folder_path;

    std::cout << "===============================================================" <<std::endl;
    std::cout << "默认程序端口："<< port <<std::endl;
    std::cout << "默认全尺寸图片路径: ./images/full_size_images" <<std::endl;
    std::cout << "默认小尺寸图片路径: ./images/half_size_images" <<std::endl;
    std::cout << "你可以在启动程序时设置参数： ./appname port allowed_token domain full_folder_path half_folder_path" <<std::endl;
    std::cout << "EXAMPLE： ./appname 9090 MY_SECRETS my_domain.com "<< "./folder1 " << "./folder2" <<std::endl;
    std::cout << "设置端口前，请注意在服务器控制台的防火墙板块(或ssh终端)放行你设置端口" <<std::endl;
    std::cout << "===============================================================" <<std::endl;


    for (int index = 0; index < argc; index++)
    {
        //port
        if (index == 1)
        {
            try
            {
                int raw_port = stoi(argv[1]);
                if (raw_port<=65535 && raw_port>=0)
                {
                    port = raw_port;
                    Controller->set_port(port);
                }
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                return -1;
            }
        }

        //token
        if (index == 2)
        {
            TokenController->save(argv[2], argv[2]);
        }

        //domain
        if (index == 3)
        {  
            Controller->set_domain(argv[3]);
        }

        //这里就不检测路径的合法性了，注意使用
        //full_folder_path
        if (index == 4)
        {
            full_folder_path = argv[4];
        }
        //half_folder_path
        if (index == 5)
        {
            half_folder_path = argv[5];
        }
        
    }
        

    //检查图片文件夹
    std::string fp_path = full_folder_path !="" ? full_folder_path : Controller->get_full_pic_path();
    std::string hfp_path = half_folder_path !="" ? half_folder_path : Controller->get_half_pic_path();
    Controller->set_full_pic_path(fp_path);
    Controller->set_half_pic_path(hfp_path);
    std::cout << "port:" << port << std::endl;
    std::cout << "full_folder_path: " << fp_path << std::endl;
    std::cout << "half_folder_path: " << hfp_path << std::endl;

    if(!(Controller->does_pic_path_exist(fp_path)))
    {
        std::cout<< "path of full size pictures does not exist, try to create one" << std::endl;
        bool fullsize_folder_create_result = Controller->create_pic_folder(fp_path);
        
        if (fullsize_folder_create_result)
        {
             std::cout<< fp_path <<" created successfully."<< std::endl;
        }

    }

    if(!(Controller->does_pic_path_exist(hfp_path)))
    {
        bool halfsize_folder_create_result = Controller->create_pic_folder(hfp_path);
        if (halfsize_folder_create_result)
        {
             std::cout<< hfp_path <<" created successfully."<< std::endl;
        }

    }

    ImageController* image_ptr = Controller.get();
    TokenStore* token_ptr = TokenController.get();
    //启动server服务
    PocoServerApp app(port, image_ptr, token_ptr);
    return app.run(argc, argv);
}
