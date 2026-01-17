#ifndef IMAGE_CONTROLLER_H
#define IMAGE_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <unordered_set>


using namespace std;

class ImageController 
{

    public:
        
        enum class Quality {
            // 压缩质量枚举
            BROKEN = 35,   //摆烂,但看起来也不差
            LOW = 65,      // 低质量,最小体积
            MEDIUM = 75,   // 中质量 (default)
            GOOD = 85,     // 良质量
            HIGH = 95,     // 高质量
            LOSSLESS = 100 // 无损
        };

        ImageController(){}
        ~ImageController(){}

        string get_full_pic_path() const;
        string get_half_pic_path() const;

        bool set_full_pic_path(const string& fpath);
        bool set_half_pic_path(const string& hfpath);

        vector<string> get_vector_f() const;
        vector<string> get_vector_hf() const;

        bool set_port(const uint port);
        uint get_port() const;

        void set_domain(const string& domain);
        string get_domain() const;


        //get current list of half and full size pics
        // 
        unordered_map<string, vector<string>> current_list();
        bool does_pic_path_exist(const string& path);
        bool is_directory(const string& path);
        bool create_pic_folder(const string& path);
        void scan_folder(const string& path);
        
        // 新增: 扫描并返回需要压缩的文件列表
        vector<string> walk_folder();
        
        int64_t compress_picture(const std::string& source_path,
                                const std::string& dest_path,
                                Quality quality = Quality::BROKEN,
                                int64_t max_size_bytes = 0) const;
        int64_t adjust_size_to_target(const cv::Mat& image, 
                                    const std::string& dest_path,
                                    const std::string& ext,
                                    int64_t target_size,
                                    int initial_quality) const;
        
                                


    private:

        string full_pic_path = "images/full_size_images";
        string half_pic_path = "images/half_size_images";
        uint port = 8080;
        string domain = "localhost";
        vector<string> vector_hf;
        vector<string> vector_f ;
        
};


#endif