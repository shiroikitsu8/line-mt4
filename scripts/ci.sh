# Build
./scripts/build.sh

# Copy dll files if line.exe exists
if [ -e "./build/msys-linemt5.dll" ]; then    
    cp -f "./build/msys-linemt5.dll" "./dist"
    cp -f "C:\msys32\usr\bin\msys-z.dll" "./dist"
    cp -f "./lib/SDL2.dll" "./dist"

    # just strip the binary because its a release build
    strip "./dist/msys-linemt5.dll"
fi