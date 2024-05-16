import math
from dataclasses import dataclass

import numpy as np

from .types import Float32Matrix, Float32Vector, UIntMatrix


@dataclass
class SVDResult:
    U: Float32Matrix
    S: Float32Vector
    Vh: Float32Matrix


# NumPy SVD


def numpy_svd(matrix: UIntMatrix) -> SVDResult:
    svd = np.linalg.svd(matrix, full_matrices=False)
    return SVDResult(U=svd.U, S=svd.S, Vh=svd.Vh)


# Power Method SVD
# https://gist.github.com/Zhenye-Na/cbf4e534b44ef94fdbad663ef56dd333


def _power_single(
    matrix: UIntMatrix, iters: int
) -> tuple[Float32Matrix, np.float32, Float32Matrix]:
    rows, cols = matrix.shape
    x = np.random.normal(0, 1, size=cols)

    B = matrix.T.dot(matrix)
    for _ in range(iters):
        x = B.dot(x)

    v = x / np.linalg.norm(x)
    sigma = np.linalg.norm(matrix.dot(v))
    u = matrix.dot(v) / sigma

    return u.reshape(rows, 1), sigma, v.reshape(cols, 1)


def power_svd(matrix: UIntMatrix) -> SVDResult:
    rank = np.linalg.matrix_rank(matrix)
    um = np.zeros((matrix.shape[0], 1))
    sv: list[np.float32] = []
    vm = np.zeros((matrix.shape[1], 1))

    # Define the number of iterations
    delta = 0.1
    epsilon = 0.97
    lamda = 2
    iterations = int(
        math.log(4 * math.log(2 * matrix.shape[1] / delta) / (epsilon * delta))
        / (2 * lamda)
    )

    # SVD using Power Method
    for _ in range(rank):
        u, sigma, v = _power_single(matrix, iterations)
        um = np.hstack((um, u))
        sv.append(sigma)
        vm = np.hstack((vm, v))
        matrix = matrix - u.dot(v.T).dot(sigma)

    return SVDResult(
        U=um[:, 1:].astype(np.float32),
        S=np.array(sv),
        Vh=vm[:, 1:].T.astype(np.float32),
    )


# Block Power Method SVD
# https://www.emis.de/journals/ASUO/mathematics_/anale2015vol2/Bentbib_A.H.__Kanber_A..pdf


def block_power_svd(matrix: UIntMatrix) -> SVDResult:
    tol = 1000
    err = tol + 1

    rows, cols = matrix.shape
    k = min(rows, cols)

    um: Float32Matrix = np.array([])
    sm: Float32Matrix = np.array([])
    vm = np.random.normal(0, 1, (cols, rows))
    while err > tol:
        qm, rm = np.linalg.qr(matrix @ vm)
        um = qm[:, :k]
        qm, rm = np.linalg.qr(matrix.T @ um)
        vm = qm[:, :k]
        sm = rm[:k, :k]
        err = np.linalg.norm(matrix @ vm - um @ sm)

    return SVDResult(
        U=um.astype(np.float32),
        S=np.diag(sm).astype(np.float32),
        Vh=vm.T.astype(np.float32),
    )
