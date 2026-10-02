set_xmakever("3.0.0")

-- Home Auto Sort 2.4 was released on 2026-09-26. The CI workflow pins
-- CommonLibSSE-NG to v9.1.0 (2026-09-24) for a close, reproducible build base.
set_project("HomeAutoSort-PTBR")
set_version("2.4.0")
set_license("GPL-3.0-or-later")
set_arch("x64")
set_languages("c++23")
set_encodings("utf-8")
set_warnings("allextra")

-- Match CommonLibSSE-NG's universal prebuilt configuration so GitHub Actions
-- can use the release bundle instead of rebuilding CommonLib from scratch.
-- This still supports Skyrim AE 1.6.1170.
set_config("skyrim_se", true)
set_config("skyrim_ae", true)
set_config("skyrim_vr", true)
set_config("rex_ini", true)
set_config("rex_json", false)
set_config("rex_toml", false)
set_config("skse_xbyak", false)
set_config("skse_patch_safety", true)

includes("lib/commonlibsse-ng")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

target("HomeAutoSort")
    add_rules("commonlibsse-ng.plugin", {
        name = "HomeAutoSort",
        author = "zfroggyman; PT-BR localization adaptation by MestreUDK",
        description = "Home Auto Sort 2.4 with externalized localization support"
    })

    add_deps("commonlibsse-ng")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src", "extern")
    set_pcxxheader("src/pch.h")
