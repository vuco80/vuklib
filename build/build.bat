g++ -Wall -Wextra -Werror -Wpedantic -c ../src/Platform.cpp ^
	-I../../../SDL3/SDL3-3.2.8/x86_64-w64-mingw32/include ^
	-I../../../SDL3_image/SDL3_image-3.2.4/x86_64-w64-mingw32/include ^
	-I../../../SDL3_ttf/SDL3_ttf-3.1.0/x86_64-w64-mingw32/include
	
ar rcs libvuk.a Platform.o