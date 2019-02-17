# Copyright 2019 The Bazel Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Experimental re-implementations of Java toolchain aliases using toolchain resolution."""

load(":toolchain_utils.bzl", "find_java_runtime_toolchain", "find_java_toolchain")

def _copy_default_info(info):
    # TODO(b/123999008): should copying DefaultInfo be necessary?
    return DefaultInfo(
        default_runfiles = info.default_runfiles,
        data_runfiles = info.data_runfiles,
        files = info.files,
    )

def _java_runtime_alias(ctx):
    """An experimental implementation of java_runtime_alias using toolchain resolution."""
    toolchain = find_java_runtime_toolchain(ctx, target = ctx.attr._java_runtime)
    return [
        toolchain,
        platform_common.TemplateVariableInfo({
            "JAVA": str(toolchain.java_executable_exec_path),
            "JAVABASE": str(toolchain.java_home),
        }),
        # See b/65239471 for related discussion of handling toolchain runfiles/data.
        DefaultInfo(
            runfiles = ctx.runfiles(transitive_files = toolchain.files),
            files = toolchain.files,
        ),
    ]

java_runtime_alias = rule(
    implementation = _java_runtime_alias,
    toolchains = ["@bazel_tools//tools/jdk:runtime_toolchain_type"],
    attrs = {
        "_java_runtime": attr.label(
            default = Label("@bazel_tools//tools/jdk:legacy_current_java_runtime"),
        ),
    },
)

def _java_host_runtime_alias(ctx):
    """An experimental implementation of java_host_runtime_alias using toolchain resolution."""
    runtime = ctx.attr._runtime
    return [
        runtime[java_common.JavaRuntimeInfo],
        runtime[platform_common.TemplateVariableInfo],
        _copy_default_info(runtime[DefaultInfo]),
    ]

java_host_runtime_alias = rule(
    implementation = _java_host_runtime_alias,
    attrs = {
        "_runtime": attr.label(
            default = Label("@bazel_tools//tools/jdk:current_java_runtime"),
            providers = [
                java_common.JavaRuntimeInfo,
                platform_common.TemplateVariableInfo,
            ],
            cfg = "host",
        ),
    },
)

def _java_toolchain_alias(ctx):
    """An experimental implementation of java_toolchain_alias using toolchain resolution."""
    toolchain = find_java_toolchain(ctx, target = ctx.attr._java_toolchain)
    return struct(
        providers = [toolchain],
        # Use the legacy provider syntax for compatibility with the native rules.
        java_toolchain = toolchain,
    )

java_toolchain_alias = rule(
    implementation = _java_toolchain_alias,
    toolchains = ["@bazel_tools//tools/jdk:toolchain_type"],
    attrs = {
        "_java_toolchain": attr.label(
            default = Label("@bazel_tools//tools/jdk:legacy_current_java_toolchain"),
        ),
    },
)

# Add aliases for the legacy native rules to allow referring to both versions in @bazel_tools//tools/jdk
legacy_java_toolchain_alias = native.java_toolchain_alias
legacy_java_runtime_alias = native.java_runtime_alias
