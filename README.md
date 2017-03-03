# Paladins-D3D
paladins d3d multihack 

How to compile:
- download and install "Microsoft Visual Studio Express 2015 for Windows DESKTOP" https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx

- open PhysXUpdateLoader.vcxproj (not PhysXUpdateLoader.vcxproj.filters) with Visual Studio 2015 (Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\WDExpress.exe)
- select x86(32bit) 
- click: project -> properties -> configuration properties -> general -> character set -> change to "not set"
- compile dll, press f7 or click the green triangle

x86 compiled dll will be in paladinsd3d\Release folder

If you share your dll with others, remove dependecy on vs runtime before compiling:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)


note: dll injection is blocked by eac anticheat

