#time_now=$( date +"%Y_%m_%d_%T" )
#cppcheck --force jni/Algorithms 2>cppcheck_$time_now.log
find . -not -path "./build/*"  \( -name "*.cpp" -or -name "*.h" -or -name "*.c"  \) -exec astyle --style=kr -xk -T -S -N -L -w -Y -O -o -n -p -U -W3 {} \+
find . -not -path "./build/*"  \( -name "*.cpp" -or -name "*.h" -or -name "*.c"  \) -exec sed -i ':a;N;$!ba;s/\n\s*\n/\n\n/g' {} \;
