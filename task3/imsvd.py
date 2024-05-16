import argparse
import math
import os
from pathlib import Path

from imsvd.rgbimg import RGBImage
from imsvd.svd import block_power_svd, numpy_svd, power_svd
from imsvd.svdimg import SVDImage


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="imsvd",
        description="Compress and decompress images using SVD",
    )

    parser.add_argument("-i", "--input", help="input file path", required=True)
    parser.add_argument("-o", "--output", help="output file path", required=True)
    parser.add_argument("-r", "--ratio", help="compression ratio", type=float)
    parser.add_argument(
        "-m",
        "--method",
        help="svd computation method",
        choices=["numpy", "power", "block_power"],
    )
    parser.add_argument(
        "-d", "--decompress", action="store_true", help="set to decompress image"
    )

    args = parser.parse_args()
    if not args.decompress and (args.ratio is None or args.method is None):
        parser.error(
            "some of the following required parameters are not present: "
            "-r/--ratio, -m/--method"
        )

    if args.decompress:
        decompress(Path(args.input), Path(args.output))
    else:
        compress(Path(args.input), Path(args.output), args.ratio, args.method)


def compress(inp: Path, out: Path, ratio: float, method: str) -> None:
    raw_size = os.path.getsize(inp)
    desired_size = math.floor(raw_size / ratio)

    match method:
        case "numpy":
            svd = numpy_svd
        case "power":
            svd = power_svd
        case "block_power":
            svd = block_power_svd
        case _:
            raise ValueError("Unknown method")

    img = RGBImage.open(inp)
    SVDImage.of_rgb_image(img, desired_size, svd).save(out)


def decompress(inp: Path, out: Path) -> None:
    SVDImage.open(inp).to_rgb_image().save(out)


if __name__ == "__main__":
    main()
