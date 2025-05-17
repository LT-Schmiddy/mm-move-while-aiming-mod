# Move While Aiming

Allows Link to move with the analog while aiming in first-person.
While active, the N64 analog input (left stick by default) will be used to move Link.
Aiming will be handled by the right-stick, overriding it's usual behavior.

Does not apply to shooting galleries, or Mini-games where Link should not be able to move around.

Also allows Link to control Epona while aiming with the bow.

This mod now also includes the optional ability for Link to fire the Bow and Hookshot with the R button,
a valuable QoL improvement for when both thumbs are needed for simultaneous aiming and firing. This feature
Will come to the Zora Boomerangs and Deku Bubble in a future version.

## Tools
You'll need to install `clang`, `make`, and the latest version of Python to build this template.
* On Windows, using [chocolatey](https://chocolatey.org/) to install both is recommended. The packages are `llvm` and `make` respectively.
  * The LLVM 19.1.0 [llvm-project](https://github.com/llvm/llvm-project) release binary, which is also what chocolatey provides, does not support MIPS correctly. The solution is to install 18.1.8 instead, which can be done in chocolatey by specifying `--version 18.1.8` or by downloading the 18.1.8 release directly.
* On Linux, these can both be installed using your distro's package manager.
* On MacOS, these can both be installed using Homebrew. Apple clang won't work, as you need a mips target for building the mod code.

On Linux and MacOS, you'll need to also ensure that you have the `zip` utility installed.

## Building

If you have Python, simply running the `build_mod.py` script should take care of everything.

If not, you can follow the instructions below:

* First, you need to build the [N64Recomp](https://github.com/N64Recomp/N64Recomp) library (included) for the `RecompModTool` utility.
* Second, run `make` (with an optional job count) to build the mod code itself.
* Third, run the `RecompModTool` utility with `mod.toml` as the first argument and the build dir (`build` in the case of this template) as the second argument.
  * This will produce your mod's `.nrm` file in the build folder.
* Finally, compile your mod offline.
  * **This part is temporary and only needed while the recomp mod runtime doesn't have LuaJIT recompilation implemented**
  * Start by running the `OfflineModRecomp` utility (included in the N64Recomp repo). Pass the following arguments: `build/mod_syms.bin build/mod_binary.bin Zelda64RecompSyms/mm.us.rev1.syms.toml build/mod_recompiled.c`
  * Next, build the dynamic library from the generated `build/mod_recompiled.c` file and pass `offline_build` as an include path.
    * On Windows, you can run:
      ```
      clang-cl build/mod_recompiled.c -fuse-ld=lld -Z7 /Ioffline_build /MD /O2 /link /DLL /OUT:build/mm_recomp_mod_template-1.0.0.dll
      ```
      On Linux, you can run:
      ```
      clang build/mod_recompiled.c -shared -fvisibility=hidden -fPIC -O2 -Ioffline_build -o build/mm_recomp_mod_template-1.0.0.so
      ```
    * Make sure your mod's dynamic library and .nrm file have the same filename (besides the extension).
      * e.g. `testmod-1.0.0.nrm` and `testmod-1.0.0.dll`
