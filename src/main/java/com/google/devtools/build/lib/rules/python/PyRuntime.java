// Copyright 2017 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.devtools.build.lib.rules.python;

import com.google.devtools.build.lib.actions.Artifact;
import com.google.devtools.build.lib.actions.MutableActionGraph.ActionConflictException;
import com.google.devtools.build.lib.analysis.ConfiguredTarget;
import com.google.devtools.build.lib.analysis.PrerequisiteArtifacts;
import com.google.devtools.build.lib.analysis.RuleConfiguredTargetBuilder;
import com.google.devtools.build.lib.analysis.RuleConfiguredTargetFactory;
import com.google.devtools.build.lib.analysis.RuleContext;
import com.google.devtools.build.lib.analysis.RunfilesProvider;
import com.google.devtools.build.lib.analysis.configuredtargets.RuleConfiguredTarget.Mode;
import com.google.devtools.build.lib.collect.nestedset.NestedSet;
import com.google.devtools.build.lib.syntax.Type;
import com.google.devtools.build.lib.vfs.PathFragment;

/** Implementation for the {@code py_runtime} rule. */
public final class PyRuntime implements RuleConfiguredTargetFactory {

  @Override
  public ConfiguredTarget create(RuleContext ruleContext) throws ActionConflictException {
    NestedSet<Artifact> files =
        PrerequisiteArtifacts.nestedSet(ruleContext, "files", Mode.TARGET);
    Artifact interpreter = ruleContext.getPrerequisiteArtifact("interpreter", Mode.TARGET);
    PathFragment interpreterPath =
        PathFragment.create(ruleContext.attributes().get("interpreter_path", Type.STRING));

    // Determine whether we're pointing to an in-build target (hermetic) or absolute system path
    // (non-hermetic).
    if ((interpreter == null) == interpreterPath.isEmpty()) {
      ruleContext.ruleError(
          "exactly one of the 'interpreter' or 'interpreter_path' attributes must be specified");
    }
    boolean hermetic = interpreter != null;
    // Validate attributes.
    if (!hermetic && !files.isEmpty()) {
      ruleContext.ruleError("if 'interpreter_path' is given then 'files' must be empty");
    }
    if (!hermetic && !interpreterPath.isAbsolute()) {
      ruleContext.attributeError("interpreter_path", "must be an absolute path.");
    }

    if (ruleContext.hasErrors()) {
      return null;
    }

    PyRuntimeProvider provider =
        hermetic
            ? PyRuntimeProvider.create(files, interpreter, /*interpreterPath=*/ null)
            : PyRuntimeProvider.create(/*files=*/ null, /*interpreter=*/ null, interpreterPath);

    return new RuleConfiguredTargetBuilder(ruleContext)
        .setFilesToBuild(files)
        .addProvider(RunfilesProvider.class, RunfilesProvider.EMPTY)
        .addProvider(PyRuntimeProvider.class, provider)
        .build();
  }

}
