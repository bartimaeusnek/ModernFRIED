namespace FriedWrapper;

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[SkipLocalsInit]
public unsafe class FriedImage : IDisposable
{
    private byte* _friedImage;
    private readonly int _outSize;
    
    private delegate void FreeDelegate(void* ptr);
    private readonly FreeDelegate _free;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static FriedImage FromExisting(Span<byte> buffer) => new FriedImage(buffer);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static FriedImage FromImage(Span<byte> buffer, int xsize, int ysize, FriedFlags flags, byte compressionLevel) => new FriedImage(buffer, xsize, ysize, (int)flags, compressionLevel);
    
    private FriedImage(Span<byte> buffer)
    {
        _outSize = buffer.Length;
        _friedImage = (byte*)NativeMemory.Alloc((nuint)buffer.Length);
        _free = NativeMemory.Free;
        buffer.CopyTo(GetFriedImageData());
    }

    private FriedImage(Span<byte> buffer, int xsize, int ysize, int flags, byte quality)
    {
        fixed (byte* bufferPtr = &buffer.GetPinnableReference())
            _friedImage = FriedApiInternal.SaveFRIED(bufferPtr, xsize, ysize, flags, quality, ref _outSize);
        _free = FriedApiInternal.FreeFRIED;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public DeFriedImage DeFryImage()
    {
        FriedApiInternal.LoadFRIED(_friedImage, _outSize, out var xsize, out var ysize, out int outSize, out byte* dataout);
        return new DeFriedImage(xsize, ysize, outSize,dataout);
    }
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Span<byte> GetFriedImageData()
    {
        return _friedImage == null 
            ? throw new ObjectDisposedException(nameof(FriedImage)) 
            : new Span<byte>(_friedImage, _outSize);
    }
    
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void ReleaseUnmanagedResources()
    {
        if (_friedImage == null)
            return;
        
        _free(_friedImage);
        _friedImage = null;
    }

    public void Dispose()
    {
        ReleaseUnmanagedResources();
        GC.SuppressFinalize(this);
    }

    ~FriedImage()
    {
        ReleaseUnmanagedResources();
    }
}