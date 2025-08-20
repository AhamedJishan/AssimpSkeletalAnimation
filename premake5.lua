workspace "AssimpSkeletalAnimation"
    architecture "x86_64"
    configurations {"Debug", "Release"}

    project "AssimpSkeletalAnimation"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        systemversion "latest"
        staticruntime "off"

        targetdir "bin/%{cfg.buildcfg}"
        objdir "bin-int/%{cfg.buildcfg}"

        files {
            "src/**.h",
            "src/**.c",
            "src/**.hpp",
            "src/**.cpp"
        }

        includedirs {
            "dependencies/include",
            "src/"
        }

        libdirs {
            "dependencies/lib"
        }

        links {
            "assimp-vc143-mtd"
        }

        postbuildcommands {
            "{COPY} %{wks.location}/dependencies/lib/assimp-vc143-mtd.dll %{cfg.targetdir}"
        }

        filter "configurations:Debug"
            symbols "On"
            runtime "Debug"
            defines "DEBUG"

        filter "configurations:Release"
            symbols "off"
            runtime "Release"
            defines "NDEBUG"
            optimize "full"