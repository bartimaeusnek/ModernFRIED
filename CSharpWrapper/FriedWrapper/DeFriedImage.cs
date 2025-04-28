namespace FriedWrapper;

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[SkipLocalsInit]
public unsafe class DeFriedImage : IDisposable
{
    private byte* _deFriedImage;
    private readonly int _outSize;
    private delegate void FreeDelegate(void* ptr);
    private readonly FreeDelegate _free;

    public int xSize;
    public int ySize;
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static DeFriedImage FromImage(Span<byte> buffer) => new DeFriedImage(buffer);

    private DeFriedImage(Span<byte> buffer)
    {
        _outSize = buffer.Length;
        _deFriedImage = (byte*)NativeMemory.Alloc((nuint)buffer.Length);
        _free = NativeMemory.Free;
        buffer.CopyTo(GetImageData());
    }
    
    internal DeFriedImage(int xsize, int ysize, int outSize, byte* deFriedImage)
    {
        xSize = xsize;
        ySize = ysize;
        _outSize = outSize;
        _deFriedImage = deFriedImage;
        _free = FriedApiInternal.FreeFRIED;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public FriedImage FryImage(FriedFlags flags, byte compressionLevel)
    {
        return FriedImage.FromImage(GetImageData(), xSize, ySize, flags, compressionLevel);
    }
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Span<byte> GetImageData()
    {
        return _deFriedImage == null 
            ? throw new ObjectDisposedException(nameof(FriedImage)) 
            : new Span<byte>(_deFriedImage, _outSize);
    }
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void ReleaseUnmanagedResources()
    {
        if (_deFriedImage == null)
            return;
        
        _free(_deFriedImage);
        _deFriedImage = null;
    }

    public void Dispose()
    {
        ReleaseUnmanagedResources();
        GC.SuppressFinalize(this);
    }

    ~DeFriedImage()
    {
        ReleaseUnmanagedResources();
    }
}