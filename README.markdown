### Project2：基于LSB数字水印的图片泄露检测系统

#### 1. 项目目标

本项目旨在开发一个基于C++的数字水印工具，实现以下核心功能：

1. **水印嵌入 (Embed):** 将一段指定的文本信息作为数字水印，通过最低有效位（LSB）算法嵌入到一张BMP格式的图片中，生成一张带水印的图片。
2. **水印提取 (Extract):** 从一张带水印的图片中，提取出隐藏的文本信息。
3. **鲁棒性测试 (Test):** 对带水印的图片施加一系列常见的图像处理攻击（如翻转、裁剪、亮度/对比度调整等），并评估在这些攻击下水印信息是否还能被成功提取。

通过本项目，可以深入理解LSB数字水印的原理、实现细节及其在实际应用中的优缺点，特别是其在鲁棒性方面的脆弱性。

#### 2. 技术原理

本项目采用**最低有效位（Least Significant Bit, LSB）**隐写术。

该技术通过**修改图像像素颜色分量（RGB）的最低一个比特位**来**嵌入信息。**

##### 嵌入过程：

1. 将需要嵌入的文本信息转换为二进制比特流。
2. 在文本流的末尾添加一个特殊的结束标记（如一串连续的0）。
3. 按顺序遍历图像的每个像素的每个颜色通道（R, G, B）。
4. 将每个颜色通道值的最低位替换为信息比特流中的对应比特。
5. 当信息全部嵌入后，保存修改后的图像。

##### 提取过程：

1. 按相同顺序遍历带水印图像的像素及其颜色通道。
2. 提取每个颜色通道值的最低有效位。
3. 将提取出的比特流重新组合。
4. 当检测到结束标记时，停止提取，并将组合好的比特流转换回文本。

#### 3. 环境依赖与编译

##### 依赖库：**CImg Library——**一个强大的C++图像处理库。

##### 编译指令

本项目代码已整合到一个文件中 `src/digital_watermark.cpp`。

```bash
g++ -o watermark_app.exe src/digital_watermark.cpp -Iinclude -O2 -static -lgdi32
```

#### 4.文件结构

```bash
image-watermarking-project2/
├── images/                 # 存放原始图片、水印图片和测试图片
│   └── original.png        # 示例原始图片
├── output/                 # 存放嵌入水印后和经过攻击后的图片
├── src/                    # 存放源代码
│   └── digital_watermark.cpp
├── include/                # 存放头文件
│   └── CImg.h
├── docs/                   # 存放项目说明文档
│   └── digital_watermarking_report.md
├── README.md               # 项目总说明
```

#### 5. 项目编译与运行指令详解

本项目是一个命令行工具，所有操作都在终端（如Windows的PowerShell或CMD）中完成。

##### 第一步：编译项目 

```
g++ -o watermark_app.exe src/digital_watermark.cpp -Iinclude -O2 -static -lgdi32
```

**指令解释：**

- `-o watermark_app.exe`: `-o` 表示输出(output)，`watermark_app.exe` 是给生成的可执行程序起的名字。
- `src/digital_watermark.cpp`: 指定要编译的源文件。
- `-Iinclude`: `-I` (大写的i) 是最重要的参数之一，它告诉编译器：“请到项目根目录下的 `include` 文件夹里去寻找头文件（比如 `CImg.h`）”。
- `-lgdi32`: 链接Windows系统自带的图形设备接口库(`gdi32`)，CImg库有时需要它来处理图片。

执行结果：

如果编译成功，终端不会显示任何信息。之后会发现项目根目录下多了一个 watermark_app.exe 文件。

##### 第二步：嵌入水印

这是项目的核心功能之一。将一段秘密文本嵌入到一张原始图片中。

**指令：**

```
.\watermark_app.exe embed .\images\original.bmp .\output\watermarked.bmp "My secret message"
```

**指令解释：**

- `.\watermark_app.exe`: `.`代表当前目录，表示运行当前目录下的这个程序。
- `embed`: 告诉程序要执行“嵌入”操作。
- `.\images\original.bmp`: 第一个参数，指定原始图片的路径。
- `.\output\watermarked.bmp`: 第二个参数，指定嵌入水印后要保存的新图片的路径和文件名。
- `"My secret message"`: 第三个参数，想要隐藏的秘密文本。

执行结果：

终端会打印出图片加载、信息长度和嵌入成功的提示。同时，在 output 文件夹内会生成一张 watermarked.bmp 图片。

##### 第四步：提取并验证水印

验证刚刚嵌入的信息是否能被正确地读取出来。

**指令：**

```
.\watermark_app.exe extract .\output\watermarked.bmp
```

**指令解释：**

- `extract`: 告诉程序要执行“提取”操作。
- `.\output\watermarked.bmp`: 指定要从哪张图片中提取信息。

执行结果：

终端会直接打印出它从图片中读取到的隐藏信息，例如：提取到的水印信息: "My secret message"。

##### 第五步：进行鲁棒性攻击测试

生成一系列被常见图像处理操作“攻击”过的图片，用于后续分析。

**指令：**

```
.\watermark_app.exe test .\output\watermarked.bmp
```

**指令解释：**

- `test`: 告诉程序要执行“鲁棒性测试”操作。
- `.\output\watermarked.bmp`: 指定要进行攻击的带水印图片。

执行结果：

终端会打印出它生成了哪些攻击图片的提示。同时，在 output 文件夹内会生成 attack_flipped.bmp, attack_cropped.bmp, attack_compressed.jpg 等多张新图片。

##### 第六步：分析攻击结果

这是实验的最后一步，通过尝试从被攻击的图片中提取水印，来检验LSB算法的健壮性。

**指令示例 (需要对每张攻击图片执行一次)：**

```
# 尝试从被裁剪的图片中提取
.\watermark_app.exe extract .\output\attack_cropped.bmp

# 尝试从被JPEG压缩的图片中提取
.\watermark_app.exe extract .\output\attack_compressed.jpg
```

**指令解释：**

- 这与第四步的提取指令完全相同，只是目标换成了被攻击后的图片。

执行结果：

观察终端打印出的提取结果。会发现，对于某些攻击（如裁剪），可能只能提取出部分信息；而对于另一些攻击（如JPEG压缩），则会完全失败，提取出乱码。将这些观察结果记录下来，就是项目的核心结论。
