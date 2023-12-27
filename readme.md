# OpenGL个人学习实践

## 教程参照LearnOpenGL CN

### 1.克隆项目及其子模块

```bat
git clone --recursive https://github.com/WhizZest/learnOpenGL.git
```

### 2.生成vcpkg可执行文件

- windows下执行命令：
```bat
./vcpkg/bootstrap-vcpkg.bat
```

- linux/mac下执行命令：
```bash
./vcpkg/bootstrap-vcpkg.sh
```

- 

### 3.安装依赖库

#### 依赖库列表
所有的依赖库全都是vcpkg支持的跨平台开源库，可以直接使用vcpkg安装。

- windows下执行命令：
```bat
./vcpkg/vcpkg.exe install glfw3 glad[gl-api-46] assimp imgui[glfw-binding] imgui[opengl3-binding] freetype box2d portaudio libsndfile
```

- linux/mac下执行命令：
```bash
./vcpkg/vcpkg install glfw3 glad[gl-api-46] assimp imgui[glfw-binding] imgui[opengl3-binding] freetype box2d portaudio libsndfile
```

### 4.cmake构建

#### 4.1 构建Debug版本
```bat
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug  --target ALL_BUILD -j 16
```

#### 4.2 构建Release版本
```bat
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target ALL_BUILD -j 16
```