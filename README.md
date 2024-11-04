# Tiny Renderer

**Render Example**:

![gun](https://github.com/Zhytou/tinyrenderer/blob/main/example/gun.gif)

![firehydrant](./example/firehydrant.gif)

**Build & Run**:

```bash
# install glfw, glm and rapidjson
sudo apt install libglfw3-dev libglm-dev rapidjson-dev

# build and compile
mkdir build
cd build
cmake -S .. -B .
make all

# run
./main
```

**Main Features**:

- Load one or multiple .obj models with configuration data in one .json file.
- Render the loaded data physically using the OpenGL api.
- Display the render result under different camera perspectives.

**TODO List**:

- [x] PBR
- [ ] Shadow(VSSM+SSAO)
- [ ] FXAA
