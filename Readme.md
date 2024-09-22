# Learn cpp

ubuntu22.04配置c++环境 gcc13.1.0
```shell
# 安装 gcc
sudo apt install build-essential
# 添加 ppa 源
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
# 安装gcc-13和g++-13
sudo apt update
sudo apt install gcc-13 g++-13
# 设置 gcc-13 和 g++-13 优先级 数字越大优先级越高
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 13
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 13
# 查看版本
gcc --version
g++ --version
# 查看优先级配置列表并切换
sudo update-alternatives --config gcc
sudo update-alternatives --config g++
# 选择想用的版本即可

```
# 学习参考
[小彭老师-现代C++项目实战](https://space.bilibili.com/263032155/channel/collectiondetail?sid=599074)
[小彭老师-STL](https://github.com/parallel101/stl1weekend)