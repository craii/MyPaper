#include "image_controller.h"

namespace fs = std::filesystem;

/* =========================
 * Getter
 * ========================= */

string ImageController::get_full_pic_path() const
{
    return full_pic_path;
}

string ImageController::get_half_pic_path() const
{
    return half_pic_path;
}

/* =========================
 * Setter
 * ========================= */

bool ImageController::set_full_pic_path(const string& fpath)
{
    if (!does_pic_path_exist(fpath)) {
        if (!create_pic_folder(fpath)) {
            return false;
        }
    }

    if (!is_directory(fpath)) {
        return false;
    }

    full_pic_path = fpath;
    return true;
}

bool ImageController::set_half_pic_path(const string& hfpath)
{
    if (!does_pic_path_exist(hfpath)) {
        if (!create_pic_folder(hfpath)) {
            return false;
        }
    }

    if (!is_directory(hfpath)) {
        return false;
    }

    half_pic_path = hfpath;
    return true;
}

/* =========================
 * Public API
 * ========================= */

unordered_map<string, vector<string>> ImageController::current_list()
{
    unordered_map<string, vector<string>> result;

    vector_f.clear();
    vector_hf.clear();

    scan_folder(full_pic_path);
    scan_folder(half_pic_path);

    result["full"] = vector_f;
    result["half"] = vector_hf;

    return result;
}

/* =========================
 * Private helpers
 * ========================= */

bool ImageController::does_pic_path_exist(const string& path)
{
    return fs::exists(path);
}

bool ImageController::is_directory(const string& path)
{
    return fs::is_directory(path);
}

bool ImageController::create_pic_folder(const string& path)
{
    try {
        return fs::create_directories(path);
    } catch (const fs::filesystem_error& e) {
        cerr << "create folder failed: " << e.what() << endl;
        return false;
    }
}

void ImageController::scan_folder(const string& path) 
{
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        string ext = entry.path().extension().string();
        string filename = entry.path().filename().string();

        // 简单判断图片格式
        if (ext == ".jpg" || ext == ".jpeg" ||
            ext == ".png" || ext == ".JPG" ||
            ext == ".JPEG"|| ext == ".PNG") {
            
            
            if (path == full_pic_path) 
            {   
                // fs::path full_pic_path = "http://localhost:10012/" + this->get_full_pic_path() + "/" + filename;
                fs::path full_pic_path = "https://" + this->get_domain() + "/" + this->get_full_pic_path() + "/" + filename;
                std::string full_path = full_pic_path.string();

                vector<string>::iterator itr = std::find(vector_f.begin(), vector_f.end(), full_path);
                if (itr == vector_f.end())
                {
                    vector_f.push_back(full_path);
                }
            } 
            else if (path == half_pic_path) 
            {
                // fs::path half_full_pic_path = "http://localhost:10012/" + this->get_half_pic_path() + "/" + filename;
                fs::path half_full_pic_path = "https://" + this->get_domain() + "/" + this->get_half_pic_path() + "/" + filename;
                std::string half_path = half_full_pic_path.string();
                vector<string>::iterator itr = std::find(vector_hf.begin(), vector_hf.end(), half_path);
                if (itr == vector_hf.end())
                {
                    vector_hf.push_back(half_path);
                }
            }
        }
    }
}



/**
 * 压缩图片到指定路径
 * @param source_path 源图片路径
 * @param dest_path 目标路径
 * @param quality 压缩质量（默认GOOD）
 * @param max_size_bytes 最大目标大小（可选，字节）
 * @return 实际压缩后的大小（字节），-1表示失败
 */
int64_t ImageController::compress_picture(const std::string& source_path,
                            const std::string& dest_path,
                            Quality quality,
                            int64_t max_size_bytes) const
{
    try {
        // 1. 读取图片（保持原始通道和位深）
        cv::Mat image = cv::imread(source_path, cv::IMREAD_UNCHANGED);
        // 保存缩小后的图片，使其与app列表图片尺寸相同375*220
        cv::Mat image_resized_to_list_cell_size;
        if (image.empty()) {
            std::cerr << "[ERROR] Failed to load image: " << source_path << std::endl;
            return -1;
        }

        // 2. 获取文件扩展名并确定压缩参数
        std::string ext = fs::path(dest_path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // 如果目标路径没有扩展名，使用源文件的扩展名
        if (ext.empty()) {
            ext = fs::path(source_path).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }

        // 3. 准备压缩参数
        std::vector<int> compression_params;
        int quality_value = static_cast<int>(quality);

        // 根据格式设置压缩参数
        if (ext == ".jpg" || ext == ".jpeg") {
            // JPEG: 使用IMWRITE_JPEG_QUALITY参数
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(quality_value);
            
            // 移除Alpha通道（JPEG不支持）
            if (image.channels() == 4) {
                cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
            }
            
            // 对于CMYK图像，转换为RGB
            if (image.channels() == 4 && image.type() == CV_8UC4) {
                cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
            }
            
        } else if (ext == ".png") {
            // PNG: 使用IMWRITE_PNG_COMPRESSION参数（0-9，9最高压缩率）
            // 质量映射：GOOD->6, HIGH->3, MEDIUM->7, LOW->9
            int png_level = 6; // 默认中等压缩
            switch (quality) {
                case Quality::HIGH:    png_level = 3; break;
                case Quality::GOOD:    png_level = 6; break;
                case Quality::MEDIUM:  png_level = 7; break;
                case Quality::LOW:     png_level = 9; break;
                case Quality::BROKEN:  png_level = 9; break;
                case Quality::LOSSLESS: png_level = 9; break;
            }
            compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(png_level);
            
            // PNG支持更多优化参数
            compression_params.push_back(cv::IMWRITE_PNG_STRATEGY);
            compression_params.push_back(cv::IMWRITE_PNG_STRATEGY_RLE);
            
        } else if (ext == ".webp") {
            // WebP: 使用IMWRITE_WEBP_QUALITY参数
            compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
            compression_params.push_back(quality_value);
            
            // WebP特定优化
            // compression_params.push_back(cv::IMWRITE_WEBP_METHOD);
            compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
            compression_params.push_back(6); // 0-6，6最高质量
            
        } else if (ext == ".tiff" || ext == ".tif") {
            // TIFF: 无损压缩
            compression_params.push_back(cv::IMWRITE_TIFF_COMPRESSION);
            compression_params.push_back(5); // LZW压缩
        } else {
            // 未知格式，默认使用JPEG格式
            std::string new_dest = dest_path + ".jpg";
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(quality_value);
            
            if (image.channels() == 4) {
                cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
            }
            

            bool success = cv::imwrite(new_dest, image, compression_params);
            if (!success) {
                std::cerr << "[ERROR] Failed to save compressed image" << std::endl;
                return -1;
            }
            
            return fs::file_size(new_dest);
        }

        // 4. 保存压缩后的图片
        //w * h = 375 * 220
        cv::resize(image, image_resized_to_list_cell_size, cv::Size(375, 220));
        bool success = cv::imwrite(dest_path, image_resized_to_list_cell_size, compression_params);
        if (!success) {
            std::cerr << "[ERROR] Failed to save compressed image" << std::endl;
            return -1;
        }

        // 5. 如果指定了最大大小，进行调整压缩
        if (max_size_bytes > 0) {
            int64_t current_size = fs::file_size(dest_path);
            if (current_size > max_size_bytes) {
                return adjust_size_to_target(image_resized_to_list_cell_size, dest_path, ext, max_size_bytes, quality_value);
            }
        }

        // 6. 输出压缩结果信息
        try {
            int64_t original_size = fs::file_size(source_path);
            int64_t compressed_size = fs::file_size(dest_path);
            double reduction = (1.0 - static_cast<double>(compressed_size) / original_size) * 100.0;
            
            std::cout << "[INFO] Compression successful:\n"
                        << "  Original: " << original_size << " bytes\n"
                        << "  Compressed: " << compressed_size << " bytes\n"
                        << "  Reduction: " << std::fixed << std::setprecision(1) 
                        << reduction << "%\n";
        } catch (...) {
            // 忽略统计信息获取失败
        }

        return fs::file_size(dest_path);

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] compress_picture: " << e.what() << std::endl;
        return -1;
    }
}


int64_t ImageController::adjust_size_to_target(const cv::Mat& image, 
                                  const std::string& dest_path,
                                  const std::string& ext,
                                  int64_t target_size,
                                  int initial_quality) const
    {
        cv::Mat working_image = image.clone();
        int current_quality = initial_quality;
        double scale = 1.0;
        const double min_scale = 0.2;  // 最小缩放到20%
        const int max_iterations = 30; // 最大迭代次数
        
        int iteration = 0;
        while (iteration < max_iterations) {
            iteration++;
            
            // 根据当前缩放比例调整图片尺寸
            if (scale < 1.0) {
                cv::resize(working_image, working_image, cv::Size(), scale, scale, 
                           cv::INTER_AREA); // 使用INTER_AREA实现下采样
            }
            
            // 构建压缩参数
            std::vector<int> params;
            
            if (ext == ".jpg" || ext == ".jpeg") {
                params.push_back(cv::IMWRITE_JPEG_QUALITY);
                params.push_back(current_quality);
            } else if (ext == ".png") {
                int png_level = std::min(9, static_cast<int>((100 - current_quality) / 10));
                params.push_back(cv::IMWRITE_PNG_COMPRESSION);
                params.push_back(png_level);
            } else if (ext == ".webp") {
                params.push_back(cv::IMWRITE_WEBP_QUALITY);
                params.push_back(current_quality);
            }
            
            // 保存并检查大小
            if (cv::imwrite(dest_path, working_image, params)) {
                int64_t current_size = fs::file_size(dest_path);
                
                if (current_size <= target_size) {
                    std::cout << "[INFO] Target size achieved after " << iteration 
                              << " iterations (scale: " << scale 
                              << ", quality: " << current_quality << ")\n";
                    return current_size;
                }
            }
            
            // 动态调整策略
            if (iteration % 2 == 0) {
                // 降低质量
                current_quality = std::max(50, current_quality - 5);
            } else {
                // 缩小尺寸
                scale = std::max(min_scale, scale - 0.1);
            }
            
            if (current_quality == 50 && scale == min_scale) {
                break; // 已经达到最小值
            }
        }
        
        std::cerr << "[WARNING] Could not achieve target size of " << target_size 
                  << " bytes, returning best effort\n";
        return fs::file_size(dest_path);
    }

vector<string> ImageController::get_vector_f() const
{
    return this->vector_f;
}

vector<string> ImageController::get_vector_hf() const
{
    return this->vector_hf;
}

bool ImageController::set_port(const uint port)
{
    if (port>0 && port <=65535)
    {
        this->port = port;
        return true;
    }
    else
    {
        return false;
    }
}

uint ImageController::get_port() const
{
    return this->port;
}

string ImageController::get_domain() const
{
    return this->domain;
}

void ImageController::set_domain(const string& domain)
{
    this->domain = domain;
}


// 在 image_controller.cpp 文件末尾添加以下实现

/**
 * 扫描 full_pic_path 和 half_pic_path,找出需要压缩的文件
 * @return 返回 full_pic_path 中存在但 half_pic_path 中不存在对应 half_* 文件的完整路径列表
 */
vector<string> ImageController::walk_folder()
{
    namespace fs = std::filesystem;
    vector<string> result;
    
    // 检查路径是否存在
    if (!fs::exists(full_pic_path) || !fs::is_directory(full_pic_path)) {
        cerr << "[ERROR] full_pic_path does not exist or is not a directory: " 
             << full_pic_path << endl;
        return result;
    }
    
    if (!fs::exists(half_pic_path) || !fs::is_directory(half_pic_path)) {
        cerr << "[WARNING] half_pic_path does not exist, creating it: " 
             << half_pic_path << endl;
        create_pic_folder(half_pic_path);
    }
    
    // 支持的图片扩展名
    unordered_set<string> valid_extensions = { ".jpg", ".jpeg", ".png", ".JPG", ".JPRG", ".PNG"};
    
    // 构建 half_pic_path 中已存在文件的集合(不含扩展名)
    unordered_set<string> half_files;
    for (const auto& entry : fs::directory_iterator(half_pic_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        
        string filename = entry.path().filename().string();
        string stem = entry.path().stem().string();
        string ext = entry.path().extension().string();
        
        // 转换为小写以便比较
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        // 检查是否以 "half_" 开头并且是图片格式
        if (valid_extensions.count(ext) && stem.rfind("half_", 0) == 0) {
            // 去掉 "half_" 前缀,保存原始文件名(不含扩展名)
            string original_stem = stem.substr(5); // 移除 "half_" 前缀
            half_files.insert(original_stem);
        }
    }
    
    // 遍历 full_pic_path,找出缺失的文件
    for (const auto& entry : fs::directory_iterator(full_pic_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        
        string filename = entry.path().filename().string();
        string stem = entry.path().stem().string();
        string ext = entry.path().extension().string();
        
        // 转换为小写以便比较
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        // 检查是否是图片格式
        if (valid_extensions.count(ext)) {
            // 检查对应的 half_ 文件是否存在
            if (half_files.find(stem) == half_files.end()) {
                // 不存在,添加到结果中
                result.push_back(entry.path().string());
                cout << "[INFO] Missing half version for: " << filename << endl;
            }
        }
    }
    
    cout << "[INFO] Found " << result.size() << " files needing compression" << endl;
    return result;
}