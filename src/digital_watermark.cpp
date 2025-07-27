/*
 * =====================================================================================
 *
 * Filename:  digital_watermark.cpp
 *
 * Description:  一个基于LSB的数字水印工具，使用CImg库。
 * 功能包括：嵌入文本水印、提取文本水印、生成一系列受攻击的图像用于鲁棒性测试。
 *
 *
 * To Compile:
 * g++ -o watermark_app digital_watermark.cpp -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
 *
 * =====================================================================================
 */

 #include <iostream>
 #include <string>
 #include <vector>
 #include <bitset>
 #include <stdexcept>
 
 // 包含CImg库。将CIMG_DISPLAY设为0表示我们不需要图形界面显示功能。
 #define cimg_display 0
#include "../include/CImg.h"
 
 using namespace cimg_library;
 
 // 将字符串转换为二进制比特流字符串
 std::string stringToBinary(const std::string& text) {
     std::string binaryString = "";
     for (char c : text) {
         binaryString += std::bitset<8>(c).to_string();
     }
     return binaryString;
 }
 
 // 将二进制比特流字符串转换为普通字符串
 std::string binaryToString(const std::string& binaryString) {
     std::string text = "";
     for (size_t i = 0; i < binaryString.length(); i += 8) {
         std::string byteString = binaryString.substr(i, 8);
         if (byteString.length() < 8) continue; // 忽略不完整的字节
         char c = static_cast<char>(std::bitset<8>(byteString).to_ulong());
         text += c;
     }
     return text;
 }
 
 // --- 水印嵌入函数 ---
 void embedWatermark(const char* inputFile, const char* outputFile, const std::string& watermarkText) {
     // 1. 加载原始图片
     CImg<unsigned char> image(inputFile);
     std::cout << "图片加载成功: " << inputFile << " (" << image.width() << "x" << image.height() << ")" << std::endl;
 
     // 2. 将水印文本转换为二进制流，并添加结束标记
     //    结束标记为连续8个0 (即一个NULL字符)，便于提取时识别
     std::string binaryWatermark = stringToBinary(watermarkText) + "00000000";
 
     // 3. 检查图片容量是否足够
     long imageCapacity = image.width() * image.height() * 3; // 每个像素有3个通道(R,G,B)
     if (binaryWatermark.length() > imageCapacity) {
         throw std::runtime_error("错误: 图片容量不足以嵌入完整的水印信息。");
     }
     std::cout << "水印信息长度 (含结束符): " << binaryWatermark.length() << " bits" << std::endl;
     std::cout << "图片可用容量: " << imageCapacity << " bits" << std::endl;
 
     // 4. 遍历像素并嵌入水印
     int bitIndex = 0;
     cimg_forXYC(image, x, y, c) {
         if (bitIndex < binaryWatermark.length()) {
             // 获取当前像素的颜色值
             unsigned char& pixelValue = image(x, y, c);
             
             // 获取要嵌入的比特位 ( '0' 或 '1' )
             char watermarkBit = binaryWatermark[bitIndex];
 
             // 修改像素的最低有效位 (LSB)
             if (watermarkBit == '1') {
                 pixelValue |= 1; // 设置最低位为1 (e.g., 10111010 | 00000001 = 10111011)
             } else { // watermarkBit == '0'
                 pixelValue &= ~1; // 设置最低位为0 (e.g., 10111011 & 11111110 = 10111010)
             }
             bitIndex++;
         } else {
             break; // 水印已全部嵌入
         }
     }
      if (bitIndex < binaryWatermark.length()) { // 再次检查是否完整嵌入
         std::cout << "警告: 循环结束，但水印可能未完全嵌入。" << std::endl;
     }
 
     // 5. 保存带水印的图片
     //    强烈建议保存为无损格式如BMP，以避免压缩算法破坏LSB信息
     image.save_bmp(outputFile);
     std::cout << "水印嵌入成功，已保存至: " << outputFile << std::endl;
 }
 
 // --- 水印提取函数 ---
 void extractWatermark(const char* inputFile) {
     // 1. 加载带水印的图片
     CImg<unsigned char> image(inputFile);
     std::cout << "正在从图片中提取水印: " << inputFile << std::endl;
 
     // 2. 遍历像素并提取LSB
     std::string extractedBinary = "";
     const std::string endMarker = "00000000";
     
     cimg_forXYC(image, x, y, c) {
         unsigned char pixelValue = image(x, y, c);
         // 提取最低有效位
         extractedBinary += ((pixelValue & 1) ? '1' : '0');
 
         // 检查是否提取到了结束标记
         if (extractedBinary.length() >= 8 && 
             extractedBinary.substr(extractedBinary.length() - 8) == endMarker) {
             // 去掉结束标记
             extractedBinary = extractedBinary.substr(0, extractedBinary.length() - 8);
             break;
         }
     }
 
     // 3. 将二进制转换为文本并输出
     if (extractedBinary.empty()) {
         std::cout << "未提取到任何信息或未找到结束标记。" << std::endl;
     } else {
         std::string extractedText = binaryToString(extractedBinary);
         std::cout << "提取到的水印信息: \"" << extractedText << "\"" << std::endl;
     }
 }
 
 // --- 鲁棒性测试函数 ---
 void testRobustness(const char* inputFile) {
     CImg<unsigned char> original(inputFile);
     std::string basePath = "output/attack_";
     std::cout << "正在对图片进行攻击测试: " << inputFile << std::endl;
     std::cout << "攻击后的图片将保存在 output/ 目录下..." << std::endl;
 
     // 1. 翻转攻击 (水平)
     CImg<unsigned char> flipped = original;
     flipped.mirror('x');
     std::string flippedPath = basePath + "flipped.bmp";
     flipped.save_bmp(flippedPath.c_str());
     std::cout << "已生成翻转图片: " << flippedPath << std::endl;
 
     // 2. 旋转攻击 (轻微旋转)
     CImg<unsigned char> rotated = original;
     rotated.rotate(5); // 旋转5度
     std::string rotatedPath = basePath + "rotated.bmp";
     rotated.save_bmp(rotatedPath.c_str());
     std::cout << "已生成旋转图片: " << rotatedPath << std::endl;
 
     // 3. 裁剪攻击 (裁掉右下角一部分)
     CImg<unsigned char> cropped = original;
     int newWidth = original.width() * 0.9;
     int newHeight = original.height() * 0.9;
     cropped.crop(0, 0, newWidth - 1, newHeight - 1);
     std::string croppedPath = basePath + "cropped.bmp";
     cropped.save_bmp(croppedPath.c_str());
     std::cout << "已生成裁剪图片: " << croppedPath << std::endl;
 
     // 4. 亮度/对比度调整
     CImg<unsigned char> adjusted = original;
     adjusted.normalize(0, 200); // 调整对比度
     cimg_forXYC(adjusted, x, y, c) { adjusted(x, y, c) = std::min(255, adjusted(x, y, c) + 20); } // 增加亮度
     std::string adjustedPath = basePath + "adjusted.bmp";
     adjusted.save_bmp(adjustedPath.c_str());
     std::cout << "已生成调色图片: " << adjustedPath << std::endl;
     
     // 5. JPEG有损压缩攻击
     std::string jpegPath = basePath + "compressed.jpg";
     original.save_jpeg(jpegPath.c_str(), 80); // 80%质量
     std::cout << "已生成JPEG压缩图片: " << jpegPath << std::endl;
 
 
     std::cout << "\n鲁棒性测试图片已生成完毕。" << std::endl;
     std::cout << "请手动使用 extract 命令尝试从这些图片中提取水印，例如：" << std::endl;
     std::cout << "./watermark_app extract " << croppedPath << std::endl;
 }
 
 
 // --- 主函数：解析命令行参数 ---
 int main(int argc, char* argv[]) {
     if (argc < 2) {
         std::cerr << "用法: " << std::endl;
         std::cerr << "  嵌入: " << argv[0] << " embed <输入图片> <输出图片> \"<水印文本>\"" << std::endl;
         std::cerr << "  提取: " << argv[0] << " extract <输入图片>" << std::endl;
         std::cerr << "  测试: " << argv[0] << " test <输入图片>" << std::endl;
         return 1;
     }
 
     try {
         std::string mode = argv[1];
         if (mode == "embed" && argc == 5) {
             embedWatermark(argv[2], argv[3], argv[4]);
         } else if (mode == "extract" && argc == 3) {
             extractWatermark(argv[2]);
         } else if (mode == "test" && argc == 3) {
             testRobustness(argv[2]);
         } else {
             throw std::invalid_argument("参数无效或数量不正确。");
         }
     } catch (const cimg_library::CImgException& e) {
         std::cerr << "CImg 错误: " << e.what() << std::endl;
         return 1;
     } catch (const std::exception& e) {
         std::cerr << "程序错误: " << e.what() << std::endl;
         return 1;
     }
 
     return 0;
 }
 