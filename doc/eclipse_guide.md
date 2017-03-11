# Using Eclipse (IDE)

To create an Eclipse project for DESPOT, run the following commands:

```bash
$ cd <workspace>
$ mkdir despot-eclipse; cd despot-eclipse
$ cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug ./<despot_dir>
```

Then import the project into Eclipse: 

1. Use `Menu File > Import`
2. Select `General > Existing projects into workspace`
3. Choose the root directory to be `despot-eclipse`. Keep "Copy projects into workspace" unchecked. 
4. Click "finish" and your Eclipse DESPOT project should be ready to use.
3. To compile the DESPOT library and examples, select `Menu Project > Build all in Eclipse`. After compiling, the binaries will be created under `despot-eclipse and despot-eclipse/examples`.

