# HuTao Fisher

胡桃老婆过生日说她钓鱼不得法, 还得自己跳下水抓, 我听了心想怎么能亏待老婆呢? 于是就找到闻名提瓦特的枫丹科学院, 向他们讲述了这一困难, 希望能够出手相助. 枫丹科学院那些姿势水平高到不知哪里去的院士们听了我的话, 非常的资瓷. 他们表示提瓦特人苦钓鱼久矣! 从枫丹廷的德拉萝诗小姐到须弥城的凯万先生, 都深受钓不上鱼的难题困扰. 若是凭此契机好好钻研, 结合枫丹发条机关术, 须弥生论派祖传生物统计学, 璃月炼丹仙法与来自世界外神秘的RTX3090之力, 必可炼成神机, 造福全提瓦特. 最终大家一起做出了这个全自动的HuTao Fisher, 老婆想钓鱼直接站到水边拿起鱼竿打开钓鱼辅助工具. 鱼就自己上钩啦!(x)



## ⚠重要提醒

如遇程序闪退问题，请尝试安装[VC++ Redistributable package](https://aka.ms/vs/17/release/vc_redist.x64.exe)



## 功能更新 Functional Update

#### 2024/8/14

- 再次优化了确定最佳张力区的模板匹配过程, 提高拉扯的稳定性
- 更新钓鱼模型, 提高了钓假龙的性能



#### 2024/7/25

- 把模型换成yolov8n, 以少许降低CPU推理性能为代价大幅提升mAP
- 更新了ncnn与opencv-mobile以及编译器的版本
- 用直接计算替代了牛顿法迭代, 提高了咬钩计算的稳定性



#### 2024/3/10

- 支持窗口化原神



#### 2024/2/4

- 适配沉玉谷钓鱼点与鱼
- 优化了确定最佳张力区左右边界的模板匹配过程, 基本修复了控制失败的bug
- 改进了采集抛竿-咬钩数据的方式



#### 2023/11/16

- 修复了一个在枫丹特定水域会把海浪条纹识别成控制框导致钓鱼失败的bug
- 更新模型参数, 优化了在4.1 4.2新区域钓鱼点的表现



#### 2023/8/25

- 更新模型参数, 适配枫丹鱼类, 同时炼丹技巧提升提高了好多mAP.
- 基本上把所有的可调参数都塞进了config.json
- 考虑到枫丹新加了酸桔饵, 颜色还跟果酿饵差不多, 把选饵过程饵的识别换成用模板匹配实现来解决这个问题.
- 优化了特定情境下检测咬钩时出现失败的问题(修改后尚未观察到这一错误)
- 向data.csv记录中添加了鱼咬钩时间一项.



#### 2023/1/26

- 引入config.json以将参数整合到配置文件中, 并加入了只钓某些大类的鱼的功能, 顺便简单起见加上了help.txt.
- 对应处理了main()中初始化程序可能会发生的异常



#### 2022/10/6

- 优化了钓鱼流程: 如果fisher确定当前的需要的鱼饵与上一条鱼的相同, 则跳过选饵的步骤.
- 优化了控制流程control()



#### 2022/9/4

- 修复了opencv-mobile写含bbox的图片时背景颜色错误的bug
- 调整抛竿参数



#### 2022/8/26

- 初步适配须弥鱼类:更新了神经网络的结构与参数并进行初步测试,基本在须弥能够正常工作.

  Preliminarily adapt fish in Sumeru: the structure and parameters of the neural network have been updated and preliminary tests have been carried out. Basically, the fisher can work normally in Sumeru.



## 简介 Intro

借鉴7eu7d7的工作[genshin-auto-fish](https://github.com/7eu7d7/genshin_auto_fish)的思路以及部分实现，在此基础上用C++重构了整体代码，并且进行了一些修改，做了一点微小的工作：(x)

- 用C++重构，简化部署，点开即用
- 重构抛竿判定：将其抽象为一三分类器（过近，过远，刚好）。并且在判定抛竿的部分通过一定的近似消除摄像机高度的影响。
- 删除了强化学习力度控制（因为我不会）
- 加入了对渊下宫鳐鱼的支持
- 将识别网络替换为~~[nanodet](https://github.com/RangiLyu/nanodet)~~(现在是[yolov8n](https://github.com/ultralytics/ultralytics)了)，使用[ncnn](https://github.com/Tencent/ncnn)进行推理，并进一步调试，并在自己的钓鱼数据集上~~达到了0.891的mAP~~反正很高就是了, 每一锅都在变
- 优化了一些错误情况的处理，使整体鲁棒性加强
- 实现多分辨率适配。（通过将截屏缩放到一1024\*576的中间层进行处理，因此原则上分辨率高于1024\*576即可正常运行）

---

Refer to the work [genshin-auto-fish](https://github.com/7eu7d7/genshin_auto_fish) of 7eu7d7. On this basis, I refactored the whole code with C++, and made some changes, and did a little work:

- Refactoring with C++: you can run the fisher without any prerequesits.

- Refactored the casting rod judgment: abstract it into a trinary classifier (too close, too far, ok). In addition, the effect of camera height is eliminated by a certain approximation.

- Removed the reinforcement learning power control (because I don't know how to perform it)

- Now support rays in Enkanomiya

- Replace the recognition network with [nanodet](https://github.com/RangiLyu/nanodet) and adjust, then infer with [ncnn](https://github.com/Tencent/ncnn), achieved 0.668 mAP on my own dataset

- Optimized the handling of some error cases and enhance the overall robustness

- Achieve multi-resolution adaptation. (By scaling the screen to a 1024\*576 intermediate layer for processing, so in theory if you have a resolution higher than 1024\*576 the fisher can work properly)




## 使用说明 Introduction for Use

不需要clone源代码，直接在release中下载最新版本即可。

先运行HutaoFisher.exe再运行游戏。

可在config.json中设置是否使用GPU推理(useGPU)，是否记录全部图像(logAllImgs)（错误时的图像始终会被输出，但其他图像记录可被关闭），是否记录抛竿数据以帮助优化抛竿判定(logData)（此项若为是，则抛竿失败后需Alt+Tab切出原神输入错误原因）, 以及是否钓某一大类的鱼(typeToFish). 具体config.json的含义见help.txt.

游戏界面的长宽比应为16:9（重要！）

在启动钓鱼器之前应手动走到钓鱼点，选择鱼竿并在游戏中进入钓鱼。

按Alt+V启动hutaofisher，Alt+X停止fisher。

提示音依照音阶顺序依次为：

1. 扫描钓鱼点寻找鱼
2. 选择待钓的鱼
3. 选择鱼饵
4. 准备抛竿
5. 检测咬钩
6. 控制力度条
7. 钓鱼成功

这也是单次钓鱼时程序的运行顺序。

钓鱼器出现严重错误时会发出低音蜂鸣并尝试从头开始钓鱼，若钓鱼过程中程序运行连续出现3次严重错误则钓鱼器会自动终止，需要重新按Alt+V以开始钓鱼。

可能有用的图像与记录存放在log文件夹中。

---

You don't need to clone the source code, just download the latest release.

Run HutaoFisher.exe before launch the game.

You can set in config.json whether to useGPU inference (useGPU), whether to log all images (logAllImgs) (images on error will always be output, but other image logging can be turned off), whether to log casting data to help optimize casting decisions (logData) (if this is yes, Alt+Tab will be required to cut out after a failed cast) If yes, you need to Alt+Tab to enter the reason for the error after a failed cast), and whether to catch a certain type of fish (typeToFish). See help.txt for the meaning of the specific config.json.

The game interface should have a 16:9 aspect ratio (important!).

Before starting the fisher, you should manually walk to the fishing place, select a fishing rod and enter fishing in the game.

Press Alt+V to activate hutaofisher and Alt+X to stop hutaofisher.

The beeps are in order of scale:

1. Scan fishing spots for fish

2. Choose the fish to catch

3. Choose bait

4. Prepare to cast the rod

5. Test the bite

6. Control strength bar

7. Successful fishing

This is also the order which the program runs on a single fishing process.

When a serious error occurs, the fisher will beep lowly and try to start fishing anew. If there are three successive serious errors occured during the fishing process, the fisher will automatically stop, and you need to press Alt+V again to restart fishing.

Probably useful images and data are stored in the log folder.



## 代码与编译 Code and Compilation

~~老实说，我没学过软件工程，所以我也不知道要怎么合理地构建一个C++项目（~~

~~所以不推荐从源代码构建程序：直接下载Release压缩包即可解压即用。~~

~~源代码位于src文件夹中。用cmake构建可能可以得到程序。~~

~~lib文件夹中包含了预编译的ncnn库，若成功编译，要运行程序应将ncnn.dll复制到编译得到的exe文件处。~~

~~另外程序还使用了opencv库，若要进行编译还得在电脑上预先安装opencv（或者自己改cmake配置以使用预编译库？实在不想做了orz）（真相是作者装完opencv以后才搞明白了一点如何链接预编译库）~~

~~不过fishing.cpp里面包含了一些注释掉的整体调试用的函数，main.cpp里也提供了调试的入口，如果你成功跑通了编译，可以利用这些函数做进一步的调试。~~

你可以根据workflow中的命令自行编译本项目。

至于数据集与网络训练的过程，没什么特色，代码也基本是一坨屎山（想到哪里写哪里），再考虑到挨mhy铁拳的风险，就不开源了(?)

---

~~To be honest, I haven't studied software engineering, so I don't know how to build a C++ project correctly.~~

~~So it's not recommended to build from source code: just download the Release package, unzip and use it.~~

~~The source code is located in the src folder. It may be possible to get a executive program by building with cmake.~~

~~The lib folder contains the pre-compiled ncnn library. To run the program, copy ncnn.dll to the compiled exe file.~~

~~In addition, the program uses the OpenCV library. To compile, you have to install OpencV on your computer in advance (or change the cmake configuration to use a precompiled library? I'm so tired to do that ORZ) (the fact is that the author didn't figure out how to link a precompiled library until after installing OpencV)~~

~~However, fishing.cpp contains some annotated functions for overall debugging, and main.cpp also provides a debug entry. If you achieve the compilation, they may help in further debugging.~~

You can compile this project by referring to the GitHub workflow.

As for the data set and network training process, there is no highlight, and the code is basically a mountain of shit (write as I think), and considering the risk of being beaten by mihoyo, so please let me not to open the source code (?)
