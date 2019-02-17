// Copyright 2016 The Bazel Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.testing.junit.runner.junit4;

import com.google.testing.junit.runner.util.Factory;
import com.google.testing.junit.runner.util.Supplier;
import org.junit.runner.Request;

/**
 * A factory that supplies {@link Request}.
 */
public final class RequestFactory implements Factory<Request> {
  private final Supplier<Class<?>> suiteClassSupplier;

  public RequestFactory(Supplier<Class<?>> suiteClassSupplier) {
    assert suiteClassSupplier != null;
    this.suiteClassSupplier = suiteClassSupplier;
  }

  @Override
  public Request get() {
    Request request = JUnit4RunnerBaseModule.provideRequest(suiteClassSupplier.get());
    if (request == null) {
      throw new NullPointerException();
    }
    return request;
  }

  public static Factory<Request> create(Supplier<Class<?>> suiteClassSupplier) {
    return new RequestFactory(suiteClassSupplier);
  }
}
