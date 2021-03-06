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

echo "==> Getting seL4 sel4runtime"
if [ -d "projects/sel4runtime/" ]; then
        git -C projects/sel4runtime/ pull
else
        git clone https://github.com/SEL4PROJ/sel4runtime.git projects/sel4runtime
fi

echo "==> Getting libACPI"
if [ -d "projects/Sofa/libACPI/" ]; then
        git -C projects/Sofa/libACPI/ pull
else
        git clone https://github.com/manu88/libACPI.git projects/Sofa/libACPI/
fi


echo "==> Getting lwip"
if [ -d "projects/libliwip/" ]; then
        git -C projects/libliwip/ pull
else
        git clone https://git.savannah.nongnu.org/git/lwip.git projects/libliwip/
fi

# create some links

if [ ! -f "init-build.sh" ]; then
	ln -s tools/seL4/cmake-tool/init-build.sh init-build.sh
fi

if [ ! -f "easy-settings.cmake" ]; then
	ln -s projects/Sofa/easy-settings.cmake easy-settings.cmake
fi
