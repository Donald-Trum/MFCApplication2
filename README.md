## 介绍

麟阁人才简历分析系统旨在提供一个高效、准确的简历分析和匹配功能，帮助人力资源部门快速筛选和匹配合适的人才。该系统将自动解析候选人的简历，并提取关键信息，如个人信息、教育背景、工作经历、技能等。通过对这些信息的智能分析和匹配算法，系统能够提供准确的人才匹配结果，帮助企业更快地找到合适的人才，提高招聘效率和成功率。


## 软件依赖


为了使用麟阁人才简历分析系统，您需要按照以下步骤进行安装和初始化：
1.安装 Miniconda / conda3：
下载适用于您的操作系统的 Miniconda / conda3 安装程序。
执行安装程序，并按照提示完成安装过程。
2.创建并激活新的 conda 环境：
打开终端或命令行界面。
运行以下命令创建新的 conda 环境：
```shell
conda create --name resume_analysis python=3.10
```
等待环境创建完成。
运行以下命令激活环境：
在 Linux 和 macOS 上：
```shell
conda activate resume_analysis
```
在 Windows 上：
```shell
activate resume_analysis
```
3.安装所需的 Python 库和框架：
在激活的 conda 环境中运行以下命令安装所需的 Python 库和框架：
```shell
pip install -r requirements.txt
```
4.下载模型参数：
从百度网盘上分别下载参数链接: 
1）Chatglm-6B参数：
https://pan.baidu.com/s/1pE6UYbYjpi_nWKZ1blaJ4w 提取码: 1037
下载后存入Linger/parse/net/chatglm/路径下
2）PJFCANN模型参数及数据:
https://pan.baidu.com/s/1Yuore_Dkk4B2Gjnc_bAj1w?pwd=cb9f 提取码：cb9f
解压后checkpoint文件夹和Recruitment_round1_train_20190716文件夹放在	PJFCANN/,  step1_data文件夹和train-test_data文件夹放在PJFCANN/data/路	径下
3）PJFCANN模型词嵌入:
https://ai.tencent.com/ailab/nlp/en/data/tencent-ailab-embedding-zh-d200-v0.	2.0-s.tar.gz下载压缩包后，解压放在PJFCANN/路径下

5.初始化数据库：
首先根据本地数据库信息更改主文件目录下config.py中的数据库配置信息，在终端或命令行界面中，进入麟阁人才简历分析系统的安装目录。
运行以下命令初始化数据库：
```shell
flask initdb --drop
```

## 代码调用 

通过
```shell
flask run {-p 端口号}
```
启动系统
端口号默认为5000
关于更多的使用说明，请参考我们的需求规格及用户说明书

## 参考文献及开源模型

https://arxiv.org/abs/2203.12277
https://arxiv.org/abs/2210.06155
http://arxiv.org/abs/2206.09116
https://github.com/THUDM/ChatGLM-6B
https://github.com/RUCAIBox/RecBole/issues/952
https://www.zhaopin.com
https://tianchi.aliyun.com/competition/entrance/231728/introduction
