from dataclasses import dataclass

import numpy as np

from .types import Float32Matrix, Float32Vector, UIntMatrix


@dataclass
class SVDResult:
    U: Float32Matrix
    S: Float32Vector
    Vh: Float32Matrix


def numpy_svd(matrix: UIntMatrix) -> SVDResult:
    svd = np.linalg.svd(matrix, full_matrices=False)
    return SVDResult(U=svd.U, S=svd.S, Vh=svd.Vh)
