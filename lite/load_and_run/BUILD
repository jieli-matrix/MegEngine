load("//brain/megbrain/lite:flags.bzl","pthread_select")

cc_library(
    name = "mgblar",
    copts = ["-std=c++14"],

    srcs = glob(["src/**/*.cpp"], exclude = ["src/main.cpp"]),
    hdrs = glob(["src/**/*.h"]),
    includes = ["src"],
    features = if_opt([
        "no_exceptions",
        "no_rtti",
    ]),
    defines = [
        "LITE_BUILD_WITH_MGE=1", 
    ],

    deps = ["//brain/megbrain/lite:lite_static_test"]+
        pthread_select(
            ["@com_github_gflags_gflags//:gflags_nothreads"],
            ["//external:gflags"]
        ),
    alwayslink = 1,
    visibility = ["//visibility:public"],
)

cc_megvii_binary(
    name = "load_and_run",
    copts = ["-std=c++14"],
    srcs = ["src/main.cpp"],
    features = if_opt([
        "no_exceptions",
        "no_rtti",
    ]),
    internal_deps = [":mgblar"],
    visibility = ["//visibility:public"],
)

