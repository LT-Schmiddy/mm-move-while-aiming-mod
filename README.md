# Majora's Mask: Movement While Aiming

This mod adds the ability to move with the left stick while aiming in first person. It's partially based on 2Ship's implementation, but with a few improvements. In particular, this version disables movement (and re-enables left-stick aiming) in a few cases where it would cause problems. More specifically, during:

* The Shooting Galleries and Cremia's milk run, since those mini-games aren't meant for the player to move, and forcing the player to aim with right-stick and shoot with B is uncomfortable.
* Riding Epona, for similar reasons (although, the ability to control Epona while aiming could probably be added).
* While aiming Zora boomerangs. You have to hold B to ready them, so you can't really aim with right-stick at the same time. Additionally, that isn't actually a first-person view and there's no animation playing while the player moves, so it looks awkward (especially if you use a mod to disable the black bars).

### Tools
You'll need to install `clang` and `make` to build this project.
* On Windows, using [chocolatey](https://chocolatey.org/) to install both is recommended. The packages are `llvm` and `make` respectively.
  * The LLVM 19.1.0 [llvm-project](https://github.com/llvm/llvm-project) release binary, which is also what chocolatey provides, does not support MIPS correctly. The solution is to install 18.1.8 instead, which can be done in chocolatey by specifying `--version 18.1.8` or by downloading the 18.1.8 release directly.
* On Linux, these can both be installed using your distro's package manager.
* On MacOS, these can both be installed using Homebrew. Apple clang won't work, as you need a mips target for building the mod code.

On Linux and MacOS, you'll need to also ensure that you have the `zip` utility installed.

You'll also need to build [N64Recomp](https://github.com/N64Recomp/N64Recomp) for the `RecompModTool` utility.

### Building
* First, run `make` (with an optional job count) to build the mod code itself.
* Next, run the `RecompModTool` utility with `mod.toml` as the first argument and the build dir (`build` in the case of this template) as the second argument.
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
