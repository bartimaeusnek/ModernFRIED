namespace FriedWrapper;

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static FriedApiInternal;
public static class FriedApi
{
    public static string GetSupportedFileVersion()
    {
        return Marshal.PtrToStringUTF8(getSupportedFileVersion()) ?? string.Empty;
    }
    
    public static bool EncodeImage(string inputPath, string outputPath, byte quality) => fried_encode(inputPath, outputPath, quality);
    public static bool DecodeImage(string inputPath, string outputPath) => fried_decode(inputPath, outputPath);

    public static DeFriedImage LoadImage(string path)
    {
        return DeFriedImage.FromImage(File.ReadAllBytes(path));
    }
    
    public static FriedImage LoadFried(string path)
    {
        return FriedImage.FromExisting(File.ReadAllBytes(path));
    }
}

[SkipLocalsInit]
[SuppressMessage("ReSharper", "InconsistentNaming")]
internal static unsafe partial class FriedApiInternal
{
    private const string Name = "fried_shared";

    [LibraryImport(Name, EntryPoint = "getSupportedFileVersion")]
    [SkipLocalsInit]
    public static partial nint getSupportedFileVersion();
    
    [LibraryImport(Name)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static partial bool LoadFRIED(byte* data, int size, out int xout, out int yout, out int outSize, out byte* dataout);
    
    [LibraryImport(Name)]
    public static partial byte* SaveFRIED(byte* image, int xsize, int ysize, int flags, byte quality, ref int outsize);
    
    [LibraryImport(Name)]
    public static partial void FreeFRIED(byte* allocated);

    [LibraryImport(Name, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static partial bool fried_encode(string inputPath, string outputPath, byte quality);
        
    [LibraryImport(Name, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static partial bool fried_decode(string inputPath, string outputPath);
    
    internal static void FreeFRIED(void* ptr)
    {
        FreeFRIED((byte*) ptr);
    }
}
