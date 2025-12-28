# OpenGL Graphics Engine Group Project

这是一个基于 C++ 和 OpenGL (Core Profile 3.3) 的基础图形引擎项目。项目集成了 GLFW 处理窗口、GLM 处理数学运算、ImGui 处理用户界面，以及 GLAD 加载 OpenGL 函数指针。

## 🚀 目前已实现功能

引擎目前的基础框架（Framework）已经搭建完毕，包含以下核心功能：

1. **基础渲染循环**：
* OpenGL 上下文初始化 (GLFW + GLAD)。
* 主循环处理与帧率计算 (`Application::Run`)。


2. **摄像机系统 (Camera System)**：
* 支持 FPS 风格漫游（WASD 移动，Shift 加速）。
* 鼠标右键按住进行视角旋转 (Pitch/Yaw)。
* 鼠标滚轮缩放视场角 (Zoom)。


3. **交互系统 (Interaction)**：
* **物体选择**：基于射线检测 (Raycasting) 与 AABB 包围盒的物体点选。
* **物体拖拽**：鼠标左键选中物体后，可在 XZ 平面上拖拽移动 (Ray-Plane Intersection)。
* **UI 面板**：集成 ImGui，支持在场景列表中选择物体，修改物体属性（位置、旋转、缩放、颜色）。


4. **场景管理**：
* 支持添加基础几何体（立方体）。
* 支持删除选中的物体。
* 高亮显示选中的物体。



---

## 📂 项目结构

```text
├── include/
│   ├── Application.h      // [Part A] 应用程序主逻辑，输入处理，射线检测
│   ├── Camera.h           // [Part A] 摄像机类
│   ├── SceneContext.h     // [Part A] 场景数据结构，对象管理
│   ├── ModelLoader.h      // [Part B] 模型加载接口
│   ├── GeometryUtils.h    // [Part C] 程序化几何生成接口
│   ├── Mesh.h             // [Part C] 网格类，VAO/VBO管理
│   ├── Renderer.h         // [Part C] 渲染器接口
│   ├── Shader.h           // 着色器工具类
│   └── Common.h           // 通用数据结构定义（顶点、纹理等）
├── src/
│   ├── main.cpp           // 程序入口
│   ├── Application.cpp    // 核心交互逻辑实现
│   ├── ModelLoader.cpp    // [Part B Stub] 模型加载实现（目前为桩函数）
│   ├── GeometryUtils.cpp  // [Part C Stub] 几何生成实现（目前球体为金字塔）
│   ├── Mesh.cpp           // [Part C] 网格渲染实现
│   ├── SceneContext.cpp   // 场景矩阵计算
│   ├── Shader.cpp          // 着色器加载与编译
│   └── glad.c             // GLAD OpenGL 函数加载
├── CMakeLists.txt         // CMake 构建脚本
├── .gitignore             // Git 忽略文件配置
└── assets/                // (需自行创建) 存放 shaders, models, textures

```

---

## 🛠 如何编译与运行


### 构建步骤 (CMake)

1. 在项目根目录下创建一个构建目录：
```bash
mkdir build
cd build

```


2. 生成构建文件：
```bash
cmake ..

```


3. 编译项目：
* Windows (Visual Studio): 打开生成的 `.sln` 文件并生成解决方案。
* Linux/Mac (Make): 运行 `cmake --build .`。


4. 运行程序：
确保 `assets` 文件夹（包含 shaders）在可执行文件的运行路径下。
```bash
./App

```



---

## 👨‍💻 开发分工 (Part A, B, C)

根据代码中的注释和存根（Stub），项目任务分配如下：

### 🟠 Part A: 场景管理与交互 (Scene & Interaction)

**主要负责人**：负责 `Application` 类与 `SceneContext` 类。

* **当前状态**：大部分核心逻辑已在框架中实现。
* **职责范围**：
1. **场景图管理**：维护 `SceneContext::objects` 列表，管理对象的生命周期。
2. **变换矩阵计算**：在 `SceneContext::DrawAll` 中，根据 Position/Rotation/Scale 正确计算 Model Matrix。
3. **用户输入处理**：处理键盘（WASD）和鼠标事件。
4. **射线检测算法**：
* `IntersectRayAABB`: 实现鼠标点击选择物体的算法。
* `IntersectRayPlane`: 实现鼠标拖拽物体在地面移动的算法。


5. **UI 逻辑**：完善 ImGui 的属性面板，确保数据能双向绑定。



### 🔵 Part B: 资源加载 (Model & Texture Loading)

**主要负责人**：负责 `ModelLoader` 类。

* **当前状态**：**未实现 (Stub)**。目前 `ModelLoader::LoadMesh` 仅返回一个硬编码的三角形。
* **需要实现的功能**：
1. **OBJ 文件解析**：
* 修改 `src/ModelLoader.cpp`。
* 读取 `.obj` 文件，解析顶点 (v)、法线 (vn)、纹理坐标 (vt) 和面 (f)。
* 构建正确的 `Mesh` 对象并返回。


2. **纹理加载**：
* 实现纹理图片的读取（建议使用 `stb_image.h`）。
* 将纹理数据上传至 OpenGL，生成 Texture ID。
* 将加载的纹理应用到 `SceneObject` 的 `textureId` 属性中。





### 🟢 Part C: 几何生成与渲染管线 (Geometry & Rendering)

**主要负责人**：负责 `GeometryUtils` 类与 `Mesh` 类。

* **当前状态**：**部分实现/Stub**。
* `Mesh` 类实现了基础的 VAO/VBO 绑定，但缺少纹理绑定逻辑。
* `CreateCube` 已实现。
* `CreateSphere` 是一个桩函数（目前生成的是金字塔）。


* **需要实现的功能**：
1. **程序化几何体生成**：
* 修改 `src/GeometryUtils.cpp`。
* 实现 **球体 (Sphere)** 的数学生成算法（基于经纬度划分）。
* 实现 **圆柱体 (Cylinder)** 和 **圆锥体 (Cone)** 的生成算法。


2. **网格渲染完善**：
* 修改 `src/Mesh.cpp` 中的 `Draw` 函数。
* 添加纹理绑定逻辑：在 `glDrawElements` 之前调用 `glBindTexture`。


3. **渲染优化 (可选)**：
* 优化 `setupMesh` 中的缓冲管理。
* 在 `Renderer` 类中实现更复杂的光照设置。
