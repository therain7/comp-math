from dataclasses import dataclass
from pathlib import Path
from typing import Self

import numpy as np
from PIL import Image

from .types import UInt8Matrix


@dataclass(frozen=True)
class RGBImage:
    height: int
    width: int

    r: UInt8Matrix
    g: UInt8Matrix
    b: UInt8Matrix

    @classmethod
    def open(cls, path: Path) -> Self:
        im = Image.open(path)
        im_split = im.convert("RGB").split()

        return cls(
            height=im.height,
            width=im.width,
            r=np.array(im_split[0]),
            g=np.array(im_split[1]),
            b=np.array(im_split[2]),
        )

    def save(self, path: Path) -> None:
        im_arr = np.dstack((self.r, self.g, self.b))
        Image.fromarray(im_arr, mode="RGB").save(path)
