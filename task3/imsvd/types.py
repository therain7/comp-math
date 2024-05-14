from nptyping import Float32, NDArray, Shape, UInt, UInt8

UIntMatrix = NDArray[Shape["*, *"], UInt]
UInt8Matrix = NDArray[Shape["*, *"], UInt8]
Float32Matrix = NDArray[Shape["*, *"], Float32]
Float32Vector = NDArray[Shape["*"], Float32]
