cls
g++ %1.cpp -o %1 -lopengl32 -lfreeglut -lglu32 -lgdi32 -static-libstdc++
.\%1%.exe
del %1.exe