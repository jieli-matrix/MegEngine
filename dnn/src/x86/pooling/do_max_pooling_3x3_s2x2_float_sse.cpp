/**
 * \file dnn/src/x86/pooling/do_max_pooling_3x3_s2x2_float_sse.cpp
 * MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2014-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
#include "src/x86/simd_macro/sse_helper.h"

#include "src/common/pooling/do_max_pooling_3x3_s2x2_float_def.inl"

#include "src/x86/simd_macro/sse_helper_epilogue.h"
