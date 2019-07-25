
echo "==> Getting seL4 kernel"
# SeL4 kernel repo
if [ -d "kernel" ]; then
	git -C kernel/ pull
else 
	git clone https://github.com/seL4/seL4.git kernel
fi


echo "==> Getting seL4 build tools"
# SeL4 build tools
mkdir -p tools

if [ -d "tools/seL4" ]; then
        git -C tools/seL4 pull
else 
	git clone https://github.com/seL4/seL4_tools.git tools/seL4
fi

echo "==> Getting seL4 util_libs"
if [ -d "projects/util_libs/" ]; then
	git -C projects/util_libs/ pull
else
	git clone https://github.com/seL4/util_libs.git projects/util_libs
fi

echo "==> Getting seL4 musllibc"
if [ -d "projects/musllibc/" ]; then
        git -C projects/musllibc/ pull
else 
	git clone https://github.com/seL4/musllibc.git projects/musllibc
fi


echo "==> Getting seL4 seL4_libs"
if [ -d "projects/seL4_libs/" ]; then
        git -C projects/seL4_libs/ pull
else 
	git clone https://github.com/seL4/seL4_libs.git projects/seL4_libs
fi

echo "==> Getting seL4sel4runtime.git"
if [ -d "projects/sel4runtime/" ]; then
        git -C projects/sel4runtime/ pull
else
        git clone https://github.com/SEL4PROJ/sel4runtime.git projects/sel4runtime
fi



# create some links

if [ ! -f "init-build.sh" ]; then
	ln -s tools/seL4/cmake-tool/init-build.sh init-build.sh
fi

if [ ! -f "CMakeLists.txt" ]; then
	ln -s tools/seL4/cmake-tool/default-CMakeLists.txt CMakeLists.txt
fi

