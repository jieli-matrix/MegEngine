# -*- coding: utf-8 -*-
# MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
#
# Copyright (c) 2014-2020 Megvii Inc. All rights reserved.
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
import numpy as np

import megengine
import megengine.optimizer as optimizer
from megengine import Parameter, tensor
from megengine.module import Module


class Simple(Module):
    def __init__(self):
        super().__init__()
        self.a = Parameter(1.23, dtype=np.float32)

    def forward(self, x):
        x = x * self.a
        return x


def test_sgd_momentum():
    net = Simple()

    optim = optimizer.SGD(net.parameters(), lr=1.0, momentum=0.9)
    optim.zero_grad()

    data = tensor([2.34])

    # do a step of train
    with optim.record():
        loss = net(data)
        optim.backward(loss)
    optim.step()

    np.testing.assert_almost_equal(optim._state[net.a]["momentum_buffer"].numpy(), 2.34)

    # do a step of infer
    loss = net(data)
    np.testing.assert_almost_equal(loss.numpy(), 2.34 * (1.23 - 2.34), 5)

    np.testing.assert_almost_equal(optim._state[net.a]["momentum_buffer"].numpy(), 2.34)

    # do a step of train
    optim.zero_grad()
    with optim.record():
        loss = net(data)
        optim.backward(loss)
    optim.step()

    np.testing.assert_almost_equal(loss.numpy(), 2.34 * (1.23 - 2.34), 5)
    np.testing.assert_almost_equal(
        optim._state[net.a]["momentum_buffer"].numpy(), 0.9 * 2.34 + 2.34
    )
