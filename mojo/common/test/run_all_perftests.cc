// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/perf_test_suite.h"
#include "mojo/common/test/test_support_impl.h"
#include "mojo/public/tests/test_support_private.h"
#include "mojo/system/embedder/embedder.h"

int main(int argc, char** argv) {
  mojo::embedder::Init();
  mojo::test::TestSupport::Init(new mojo::test::TestSupportImpl());
  return base::PerfTestSuite(argc, argv).Run();
}
