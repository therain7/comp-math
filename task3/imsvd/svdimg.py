import struct
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import Self

import numpy as np
from numpy.lib import math

from .rgbimg import RGBImage
from .svd import SVDResult
from .types import UInt8Matrix

INT_SIZE = 4
FLOAT_SIZE = 4


@dataclass(frozen=True)
class SVDImage:
    height: int
    width: int
    k: int

    r: SVDResult
    g: SVDResult
    b: SVDResult

    @classmethod
    def of_rgb_image(
        cls,
        img: RGBImage,
        desired_size: int,
        svd: Callable[[UInt8Matrix], SVDResult],
    ) -> Self:
        k = math.floor(
            (desired_size - 3 * INT_SIZE - 5)
            / (3 * FLOAT_SIZE * (img.height + img.width + 1))
        )

        if k < 1:
            raise ValueError("Desired size is impossible without losing all data")

        def ksvd(matrix: UInt8Matrix) -> SVDResult:
            m = svd(matrix)
            return SVDResult(U=m.U[:, :k], S=m.S[:k], Vh=m.Vh[:k, :])

        return cls(
            height=img.height,
            width=img.width,
            k=k,
            r=ksvd(img.r),
            g=ksvd(img.g),
            b=ksvd(img.b),
        )

    def to_rgb_image(self) -> RGBImage:
        def svd_to_matrix(svd: SVDResult) -> UInt8Matrix:
            return (svd.U @ np.diag(svd.S) @ svd.Vh).clip(0, 255).astype(np.uint8)

        return RGBImage(
            height=self.height,
            width=self.width,
            r=svd_to_matrix(self.r),
            g=svd_to_matrix(self.g),
            b=svd_to_matrix(self.b),
        )

    @classmethod
    def open(cls, path: Path) -> Self:
        with open(path, "rb") as f:
            # header
            if f.read(5) != b"IMSVD":
                raise ValueError("Provided file is not an IMSVD file")
            height, width, k = struct.unpack("<III", f.read(3 * INT_SIZE))

            def read_svd_result() -> SVDResult:
                U = np.frombuffer(
                    f.read(height * k * FLOAT_SIZE), dtype=np.float32
                ).reshape(height, k)

                S = np.frombuffer(f.read(k * FLOAT_SIZE), dtype=np.float32)

                Vh = np.frombuffer(
                    f.read(k * width * FLOAT_SIZE), dtype=np.float32
                ).reshape(k, width)

                return SVDResult(U=U, S=S, Vh=Vh)

            # data
            r = read_svd_result()
            g = read_svd_result()
            b = read_svd_result()

            return cls(height=height, width=width, k=k, r=r, g=g, b=b)

    def save(self, path: Path) -> None:
        with open(path, "wb") as f:

            def write_svd_result(svd: SVDResult) -> None:
                f.write(svd.U.astype(np.float32).tobytes())
                f.write(svd.S.astype(np.float32).tobytes())
                f.write(svd.Vh.astype(np.float32).tobytes())

            # header
            f.write(b"IMSVD")
            f.write(struct.pack("<III", self.height, self.width, self.k))

            # data
            write_svd_result(self.r)
            write_svd_result(self.g)
            write_svd_result(self.b)
