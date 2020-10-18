# sel4 Kernel 101

This first article is about the sel4 micro kernel.
More general informations can be found here:

- <https://sel4.systems>
- <https://docs.sel4.systems/FrequentlyAskedQuestions.html>

The main purpose of this article is to create yet an other 'Hello World' project. Yes, SeL4 team already offers a build system, built around CMake & ninja, with git & google's repo as source control. The idea here is to create from scratch the code and project layout.

# sel4 Kernel components:

- the kernel itself : <https://github.com/seL4/seL4>
- the seL4 Tools, for project configuration, build, etc. : <https://github.com/seL4/seL4_tools>
- seL4 userland libraries : <https://github.com/seL4/seL4_libs>
- seL4 util libraries :  <https://github.com/seL4/util_libs>

Note : this article follows and is inspirated by this page :
<https://docs.sel4.systems/Developing/Building/Incorporating>

Let's call our project 'Femur' which, if you know French, est un _os_.

SeL4 kernel must be integrated into each project you create, this is not a dynamic library what so ever, even if you probably might sym-link it from a global folder somewhere.

for the rest of this article, we will assume 'Femur' is our root directory.

The global project layout will always looks like this :

```
Femur/
├── kernel/
├── projects/
│	 ├── Hello/
│	 ├── musllibc/
│	 ├── utils_libs/
│  └── seL4_libs/
├── tools/  
   └── cmake-tool/
```
The 'kernel' folder contains, well kernel code, and we have no reasons for now to touch it.
The 'projects' folder will contain all our custom code for userland applications, libraries, etc.
The 'tools' folder contains the tools for building the project.

# Project Preparation

inside the root folder (i.e. Femur):

```bash
# Clone seL4 into kernel dir

git clone https://github.com/seL4/seL4.git kernel

# Clone seL4_tools repo into tools dir
git clone https://github.com/seL4/seL4_tools.git tools

# Create 'projects' folder
mkdir projects

# Clone seL4_libs repo into projects/seL4_libs
git clone https://github.com/seL4/seL4_libs.git projects/seL4_libs

# Clone the musl libc into projects/musllibc
git clone https://github.com/seL4/musllibc.git projects/musllibc

# Clone util_libs repo into projects/util_libs
git clone https://github.com/seL4/util_libs.git projects/util_libs

# seL4_tools contains an usefull script : 'init-build.sh'. Let's sym-link it in our root directory.
ln -s tools/cmake-tool/init-build.sh init-build.sh

```

Now we need to setup the build chain. Since seL4 is using CMake, we'll later create a 'build' something folder, but we need a valid CMakeLists.txt in the root project folder. Sel4's documentation sugest to symlink the default CMakeLists.txt found in seL4_tools, but we are going to create our own, with the following content:

```CMake
cmake_minimum_required(VERSION 3.7.2)

if (${PLATFORM} IN_LIST KernelX86Sel4Arch_all_strings)
        set(KernelArch x86 CACHE STRING "" FORCE)
        set(KernelX86Sel4Arch ${PLATFORM} CACHE STRING "" FORCE)
endif()

include(tools/cmake-tool/default-CMakeLists.txt)

if(SIMULATION)
        ApplyCommonSimulationSettings("x86")
else()
        if(KernelArchX86)
                set(KernelIOMMU ON CACHE BOOL "" FORCE)
        endif()
endif()

# We must build the debug kernel because the tutorials rely on seL4_DebugPutChar
# and they don't initialize a platsupport driver.
ApplyCommonReleaseVerificationSettings(FALSE FALSE)

GenerateSimulateScript()
```

Check the file here : <https://github.com/manu88/SeL4_101/blob/master/CMakeLists.txt>


We're creating a real file instead of a symbolic link because this is a good place to set global build variables & environment settings. Most sel4 projects ends up setting kernel related configuration in files inside the 'projects' folder; I just think this is a more convenient and clean place.

Let's try to build
```bash
mkdir build
cd build
../init-build.sh  -DPLATFORM=x86_64 -DSIMULATION=TRUE
```
You should see a lot of repeated errors like :

```
Error evaluating generator expression:

    $<TARGET_PROPERTY:rootserver_image,ROOTSERVER_IMAGE>

  Target "rootserver_image" not found.
```

This is because sel4 buildchain excepts some user application to run after kernel initialization (an equivalent to init/LaunchD). This application is called the root Server. 

So we need to write some custom code. And it's about time ! 

Let's add a new folder called 'Hello' inside the projects directory. This will be our root server.

```
mkdir projects/Hello/
```

So far, our project should look like this:

```
Femur/  
├── kernel/  
├── projects/  
│	├── Hello/  
│	├── musllibc/  
│	├── utils_libs/  
│ └── seL4_libs/  
├── build/  
├── tools/  
│   └── cmake-tool/  
│   └── cmake-tool/  
├── init-build.sh   
├── CMakeLists.txt  

```
Our project needs a CMakeLists.txt file and some C sources as well

#  Create the source folder

```bash
mkdir projects/Hello/src
```

Inside projects/Hello/src, we add the simpliest program :

```C
#include <stdio.h>

int main(void)
{
    printf("Salut, Monde!\n");
    return 0;
}
```

And we add a CMakeLists.txt inside projects/Hello :

```
cmake_minimum_required(VERSION 3.7.2)

project(Hello C) # create a new C project called 'Hello' 


add_executable(Hello src/main.c) # add files to our project. Paths are relative to this file.

target_link_libraries(Hello sel4muslcsys  muslc) # we need to link against the standard C lib for printf


# Set this image as the rootserver
DeclareRootserver(Hello)
```

Now we have everything we need ! We just have to go inside the build folder:

```bash
# First we remove everything from the folder
rm -r *

# Re-run the initialization script :
../init-build.sh  -DPLATFORM=x86_64 -DSIMULATION=TRUE

# Then start the build process
ninja

#If everything goes well, we can start the Simulation,

./simulate
```
Buried inside the boot/kernel log, You should see :

```
Moving loaded userland images to final location: from=0xa50000 to=0xa11000 size=0x11d000
Starting node #0 with APIC ID 0
Mapping kernel window is done
Booting all finished, dropped to user space
Bonjour, Monde
Warning: using printf before serial is set up. This only works as your
printf is backed by seL4_Debug_PutChar()
Brintf is back
seL4 root server abort()ed
Debug halt syscall from user thread 0xffffff801ffb5400 "rootserver"
halting...
Kernel entry via Unknown syscall, word: 523
```

Yay!

You can exit qemu with ctl-a + x.

