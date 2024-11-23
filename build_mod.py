import pathlib, subprocess, os, tomllib
import build_n64recomp_tools as bnt

mod_data = tomllib.loads(pathlib.Path("mod.toml").read_text())["manifest"]
# print(mod_data)


deps = bnt.get_dependencies()

subprocess.call(deps["make"])

subprocess.call(
    [
        # "./tools/RecompModTool.exe",
        bnt.get_RecompModTool_path(),
        "mod.toml",
        "build"
    ],
    cwd=os.getcwd()
)

subprocess.call(
    [
        # "./tools/OfflineModRecomp.exe",
        bnt.get_OfflineModRecomp_path(),
        "build/mod_syms.bin",
        "build/mod_binary.bin",
        "Zelda64RecompSyms/mm.us.rev1.syms.toml",
        "build/mod_recompiled.c",
    ],
    cwd=os.getcwd()
)

# Compile DLL:
subprocess.call(
    [
        deps["clang-cl"], 
        "build/mod_recompiled.c", 
        "-fuse-ld=lld", 
        "-Z7",
        "/Ioffline_build",
        "/Imm-decomp/include",
        "/Imm-decomp/overlays/gamestates/ovl_file_choose/",
        "/MD",
        "/O2",
        "/link",
        "/DLL", 
        f"/OUT:build/{mod_data['id']}-{mod_data['version']}.dll"
    ]
)